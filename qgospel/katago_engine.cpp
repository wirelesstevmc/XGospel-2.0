#include "katago_engine.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>

KataGoEngine::KataGoEngine(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_state(IDLE)
    , m_startup_timer(new QTimer(this))
    , m_komi(6.5)
    , m_handicap(0)
    , m_boardsize(19)
    , m_time_per_move(5)
    , m_init_acks_remaining(0)
    , m_clear_board_pending(false)
    , m_stop_for_score(false)
    , m_awaiting_ownership(false)
{
    m_startup_timer->setSingleShot(true);
    m_startup_timer->setInterval(60000);  // 60s — allows time for SSH connections

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &KataGoEngine::onStdoutReady);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &KataGoEngine::onStderrReady);
    connect(m_process, &QProcess::started, [this]() {
        qDebug() << "[KataGoEngine] QProcess::started fired — PID:" << m_process->processId();
        emit gtpLogLine("--- Process started (PID " +
                        QString::number(m_process->processId()) + ") ---", false);
    });
    connect(m_process,
            static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            this, &KataGoEngine::onProcessError);
    connect(m_process,
            static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &KataGoEngine::onProcessFinished);
    connect(m_startup_timer, &QTimer::timeout,
            this, &KataGoEngine::onStartupTimeout);
}

KataGoEngine::~KataGoEngine()
{
    stop();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void KataGoEngine::start(const QString &enginePath,
                         const QString &engineArgs,
                         const QString &workingDir,
                         double komi,
                         int    handicap,
                         int    boardsize,
                         int    timePerMove)
{
    if (m_state != IDLE) {
        qWarning() << "[KataGoEngine] start() called while not IDLE";
        return;
    }

    m_komi          = komi;
    m_handicap      = handicap;
    m_boardsize     = (boardsize > 0) ? boardsize : 19;
    m_time_per_move = (timePerMove > 0) ? timePerMove : 5;
    m_stdout_buf.clear();
    m_cmd_queue.clear();
    m_pending_cmd.clear();

    m_process->setWorkingDirectory(workingDir);

    // Build argument list: engineArgs may be empty or a space-separated string
    QStringList args;
    if (!engineArgs.trimmed().isEmpty())
        args = engineArgs.trimmed().split(' ', QString::SkipEmptyParts);

    qDebug() << "[KataGoEngine] Starting:" << enginePath << args
             << "workdir:" << workingDir;
    emit gtpLogLine(QString("--- Launching: %1 %2 ---").arg(enginePath).arg(args.join(' ')), true);

    setState(STARTING);
    m_startup_timer->start();
    m_process->start(enginePath, args);
}

void KataGoEngine::stop()
{
    m_startup_timer->stop();
    if (m_process->state() != QProcess::NotRunning) {
        // Send quit gracefully
        m_process->write("quit\n");
        m_process->waitForFinished(2000);
        if (m_process->state() != QProcess::NotRunning)
            m_process->kill();
    }
    m_cmd_queue.clear();
    m_pending_cmd.clear();
    setState(IDLE);
}

void KataGoEngine::sendPlay(StoneColor color, int x, int y)
{
    enqueue(QString("play %1 %2").arg(colorToGtp(color)).arg(coordToGtp(x, y)));
}

void KataGoEngine::sendPass(StoneColor color)
{
    enqueue(QString("play %1 pass").arg(colorToGtp(color)));
}

void KataGoEngine::requestGenmove(StoneColor color)
{
    enqueue(QString("genmove %1").arg(colorToGtp(color)));
}

void KataGoEngine::requestFinalScore()
{
    if (m_state == THINKING) {
        // KataGo is mid-genmove; send stop immediately (out-of-band) to interrupt it.
        // The stop response will be a move/pass which we discard via m_stop_for_score.
        m_stop_for_score = true;
        qDebug() << "[KataGoEngine] Sending stop before final_score";
        emit gtpLogLine(">>> stop", true);
        m_process->write("stop\n");
        m_cmd_queue.prepend("final_score");  // goes next after stop ACK
    } else {
        enqueue("final_score");
    }
}

void KataGoEngine::requestOwnership()
{
    // Guard: if already in flight, do not send a second request.
    if (m_awaiting_ownership) {
        qDebug() << "[KataGoEngine] requestOwnership called while already in flight — ignoring";
        return;
    }
    m_awaiting_ownership = true;
    m_latest_ownership.clear();

    // kata-raw-nn 0: synchronous single-response command — returns whiteOwnership
    // (361 floats) in one \n\n-terminated GTP block. Immune to pondering state
    // and visit-count scaling — always evaluates exactly once via the neural net.
    enqueue("kata-raw-nn 0");
}

void KataGoEngine::cancelOwnership()
{
    if (!m_awaiting_ownership) return;
    m_awaiting_ownership = false;
    m_latest_ownership.clear();
    qDebug() << "[KataGoEngine] cancelOwnership — cleared state";
}

void KataGoEngine::clearBoard()
{
    m_clear_board_pending = true;
    enqueue("clear_board");
}

void KataGoEngine::enqueueRaw(const QString &cmd)
{
    enqueue(cmd);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void KataGoEngine::setState(State s)
{
    m_state = s;
}

void KataGoEngine::enqueue(const QString &cmd)
{
    m_cmd_queue.enqueue(cmd);
    if (isReady() && m_pending_cmd.isEmpty())
        dispatchNext();
}

void KataGoEngine::dispatchNext()
{
    if (m_cmd_queue.isEmpty()) return;
    m_pending_cmd = m_cmd_queue.dequeue();
    qDebug() << "[KataGoEngine] >>>" << m_pending_cmd;
    emit gtpLogLine(">>> " + m_pending_cmd, true);
    m_process->write((m_pending_cmd + "\n").toUtf8());
    if (m_pending_cmd.startsWith("genmove"))
        setState(THINKING);
}

// ---------------------------------------------------------------------------
// Slots — process I/O
// ---------------------------------------------------------------------------

// Standard star-point positions for handicap stones (GTP vertex strings).
// Indexed by handicap count 2..9.  Follows IGS placement convention:
// 3-stone: upper-right corner (Q16) is left empty → stones at D4, Q4, D16.
static const char* const HANDICAP_VERTICES[10][10] = {
    {},                                                             // 0 — unused
    {},                                                             // 1 — unused
    {"D4","Q16",nullptr},                                           // 2
    {"D4","D16","Q16",nullptr},                                     // 3  IGS: Q4 empty
    {"D4","Q4","D16","Q16",nullptr},                                // 4
    {"D4","Q4","D16","Q16","K10",nullptr},                          // 5
    {"D4","Q4","D16","Q16","D10","Q10",nullptr},                    // 6
    {"D4","Q4","D16","Q16","D10","Q10","K10",nullptr},              // 7
    {"D4","Q4","D16","Q16","D10","Q10","K4","K16",nullptr},         // 8
    {"D4","Q4","D16","Q16","D10","Q10","K4","K16","K10",nullptr},   // 9
};

void KataGoEngine::onStderrReady_handleGTPReady()
{
    // Called when "GTP ready" is detected on either stderr or stdout.
    // Guards against double-firing if it appears on both.
    if (m_state != STARTING) return;

    qDebug() << "[KataGoEngine] GTP ready — starting init sequence";
    emit gtpLogLine("--- GTP ready ---", false);
    m_startup_timer->stop();

    m_init_acks_remaining = 4;  // time_settings + boardsize + komi + clear_board
    enqueue(QString("time_settings 0 %1 1").arg(m_time_per_move));
    enqueue(QString("boardsize %1").arg(m_boardsize));
    enqueue(QString("komi %1").arg(m_komi));
    enqueue("clear_board");

    if (m_handicap >= 2 && m_handicap <= 9) {
        QStringList verts;
        for (int i = 0; HANDICAP_VERTICES[m_handicap][i] != nullptr; ++i)
            verts << QString(HANDICAP_VERTICES[m_handicap][i]);
        enqueue(QString("set_free_handicap %1").arg(verts.join(' ')));
        m_init_acks_remaining++;
    }

    setState(READY);
    dispatchNext();
}

void KataGoEngine::onStderrReady()
{
    QByteArray data = m_process->readAllStandardError();
    QString text = QString::fromUtf8(data);

    // Log all stderr lines to console for debugging
    for (const QString &line : text.split('\n', QString::SkipEmptyParts))
        emit gtpLogLine("[err] " + line, false);

    if (m_state == STARTING && text.contains("GTP ready"))
        onStderrReady_handleGTPReady();
}

void KataGoEngine::onStdoutReady()
{
    QByteArray raw = m_process->readAllStandardOutput();
    QString text = QString::fromUtf8(raw);

    // SSH with -t -t merges remote stderr onto the TTY (stdout pipe).
    // Scan for "GTP ready" here too so wrapper scripts work correctly.
    if (m_state == STARTING && text.contains("GTP ready")) {
        qDebug() << "[KataGoEngine] GTP ready detected on stdout (SSH -t -t mode)";
        onStderrReady_handleGTPReady();
        // Discard everything up to and including the "GTP ready" line —
        // it's KataGo startup chatter, not GTP protocol responses.
        int pos = text.indexOf("GTP ready");
        int nl  = text.indexOf('\n', pos);
        QString remainder = (nl >= 0) ? text.mid(nl + 1) : QString();
        remainder.remove('\r');  // SSH TTY adds \r\n — strip \r
        m_stdout_buf = remainder;
        return;
    }

    // SSH TTY mode adds \r before \n — strip them so the parser sees clean \n\n
    text.remove('\r');
    m_stdout_buf += text;

    // GTP responses are terminated by a blank line ("\n\n").
    // Process all complete responses in the buffer.
    while (true) {
        // Look for a response line followed by a blank line
        int blank = m_stdout_buf.indexOf("\n\n");
        if (blank == -1) {
            // No \n\n yet — response is incomplete. Discard non-GTP prefix lines
            // (TTY echo, KataGo startup noise) but never consume a = or ? line
            // without its terminating \n\n: genmove responses like "= Q4" can arrive
            // split across TCP packets ("=\n" then "Q4\n\n"), and consuming the bare
            // "=" would lose the coordinate.
            int nl = m_stdout_buf.indexOf('\n');
            if (nl == -1) break;  // incomplete line — wait for more data
            QString line = m_stdout_buf.left(nl).trimmed();
            if (line.startsWith('=') || line.startsWith('?')) {
                // GTP response line present but \n\n not yet arrived — wait for rest
                break;
            }
            // Non-GTP line (TTY echo, info) — safe to discard and keep scanning
            m_stdout_buf = m_stdout_buf.mid(nl + 1);
            continue;
        }
        QString block = m_stdout_buf.left(blank);
        m_stdout_buf = m_stdout_buf.mid(blank + 2);
        // SSH TTY echo: the block may contain our sent command before the = response.
        // Find the GTP response line (starts with = or ?) and collect the full body.
        // KataGo sometimes sends "=\nQ4\n\n" split so the = and coordinate are on
        // separate lines within the same \n\n-delimited block.
        QString response;
        QStringList extra_body_lines;
        bool found_response = false;
        for (const QString &bline : block.split('\n')) {
            QString t = bline.trimmed();
            if (!found_response) {
                if (t.startsWith('=') || t.startsWith('?')) {
                    response = t;
                    found_response = true;
                }
                // else: SSH echo / info line before the response — skip
            } else {
                // Lines after the = / ? line are continuation body (e.g. the coordinate)
                if (!t.isEmpty()) extra_body_lines << t;
            }
        }
        if (response.isEmpty()) continue;  // no GTP response in this block — skip
        // Append continuation lines to the response body (handles both "=\nVERTEX\n\n"
        // split responses and kata-raw-nn multi-line data blocks).
        if (!extra_body_lines.isEmpty())
            response = response + " " + extra_body_lines.join(' ');

        qDebug() << "[KataGoEngine] <<<" << response.left(120);
        emit gtpLogLine("<<< " + response.left(120) + (response.size() > 120 ? "..." : ""), false);
        if (m_pending_cmd.isEmpty()) continue;
        if (response.startsWith('?')) {
            QString errMsg  = response.mid(1).trimmed();
            QString failCmd = m_pending_cmd;
            m_pending_cmd.clear();
            setState(READY);
            // If kata-raw-nn failed, unblock ownership with empty result.
            if (m_awaiting_ownership && failCmd.startsWith("kata-raw-nn")) {
                m_awaiting_ownership = false;
                m_latest_ownership.clear();
                qWarning() << "[KataGoEngine] kata-raw-nn failed — clearing ownership state:" << errMsg;
                emit ownershipReady(QVector<float>());
            }
            if (failCmd.startsWith("play ") && errMsg.contains("illegal", Qt::CaseInsensitive)) {
                // Extract the GTP vertex from the failed play command (e.g. "play B D3" → "D3")
                QStringList parts = failCmd.split(' ');
                QString vertex = (parts.size() >= 3) ? parts[2] : failCmd;
                m_cmd_queue.clear();  // cancel pending genmove — board is desynced
                emit illegalMove(vertex);
            } else {
                dispatchNext();
                emit engineError(errMsg);
            }
        } else if (response.startsWith('=')) {
            QString body = response.mid(1).trimmed();
            QString cmd  = m_pending_cmd;
            // KataGo sends an immediate empty "=\n\n" when genmove is accepted,
            // then a second "= <vertex>\n\n" when thinking is done.
            // Don't dispatch the empty ack — keep pending_cmd set and wait for real response.
            // genmove: KataGo sends "=\n\n" immediately then "= <vertex>\n\n" when done.
            // kata-raw-nn: same two-phase pattern — bare "=\n\n" then "= symmetry 0 ...\n\n".
            if (body.isEmpty() && (cmd.startsWith("genmove") || cmd.startsWith("kata-raw-nn"))) {
                qDebug() << "[KataGoEngine]" << cmd.section(' ', 0, 0) << "ack (thinking) — waiting for data";
                continue;
            }
            m_pending_cmd.clear();
            setState(READY);
            handleResponse(body);
            Q_UNUSED(cmd);
            dispatchNext();
        }
    }
}

void KataGoEngine::handleResponse(const QString &body)
{
    // Determine what command this response belongs to by examining the queue
    // history — we already cleared m_pending_cmd before calling this, so we
    // use the content of body to decide.  The caller passes the body directly.

    // We infer context from what we just sent (stored before clearing):
    // This is called immediately after m_pending_cmd is cleared, but we
    // stashed the command in a local before clearing — see onStdoutReady.
    // Here we re-route based on body content and a simple heuristic:
    // genmove responses are coordinates, pass, or resign.
    // final_score responses are "B+N" or "W+N".
    // Everything else is an ACK (empty or vertex list for handicap).

    // stop-for-score: swallow the move/pass KataGo returns after stop, then
    // dispatch final_score which was prepended to the queue.
    if (m_stop_for_score) {
        m_stop_for_score = false;
        qDebug() << "[KataGoEngine] stop-for-score: discarding interrupted move response:" << body;
        setState(READY);
        dispatchNext();
        return;
    }

    if (body.isEmpty()) {
        // User-initiated clear_board: emit boardCleared so the UI can re-enable Engine: Go
        if (m_clear_board_pending) {
            m_clear_board_pending = false;
            emit boardCleared();
            return;
        }
        // Empty ACK: boardsize / komi / clear_board / set_free_handicap / play
        if (m_init_acks_remaining > 0) {
            m_init_acks_remaining--;
            if (m_init_acks_remaining == 0) {
                // Emit the stones we placed via set_free_handicap so the board
                // can render them before the first genmove.
                if (m_handicap >= 2 && m_handicap <= 9) {
                    QList<QPair<int,int>> stones;
                    for (int i = 0; HANDICAP_VERTICES[m_handicap][i] != nullptr; ++i) {
                        int x, y;
                        if (gtpToCoord(QString(HANDICAP_VERTICES[m_handicap][i]), x, y) && x >= 0)
                            stones.append(qMakePair(x, y));
                    }
                    if (!stones.isEmpty())
                        emit handicapStonesReady(stones);
                }
                emit engineReady();
            }
        }
        return;
    }

    // kata-raw-nn response: "symmetry N whiteWin f whiteLoss f ... whiteOwnership f f ... f ..."
    // The whiteOwnership field contains boardsize*boardsize floats (+1=white, -1=black).
    // We negate to match the convention used by the rest of the bot (positive = black owns).
    if (m_awaiting_ownership && body.contains("whiteOwnership", Qt::CaseInsensitive)) {
        m_awaiting_ownership = false;
        QVector<float> parsed;
        int idx = body.indexOf("whiteOwnership", 0, Qt::CaseInsensitive);
        if (idx != -1) {
            QString after = body.mid(idx + 14).trimmed();
            QStringList tokens = after.split(QRegExp("[\\s]+"), QString::SkipEmptyParts);
            for (const QString &tok : tokens) {
                bool ok = false;
                float val = tok.toFloat(&ok);
                if (!ok) break;
                parsed.append(-val);  // negate: kata-raw-nn is white-positive; bot expects black-positive
                if (parsed.size() == 361) break;
            }
        }
        qDebug() << "[KataGoEngine] kata-raw-nn ownership:" << parsed.size() << "values";
        emit gtpLogLine(QString("<<< [kata-raw-nn ownership: %1 values]").arg(parsed.size()), false);
        emit ownershipReady(parsed.size() == 361 ? parsed : QVector<float>());
        m_latest_ownership.clear();
        return;
    }

    QString upper = body.toUpper();

    // final_score response: "B+N.N" or "W+N.N" or "0"
    if ((upper.startsWith('B') || upper.startsWith('W') || upper == "0")
        && (upper.contains('+') || upper == "0")) {
        emit scoreReady(body);
        return;
    }

    // genmove response
    if (upper == "PASS") {
        emit passMoveReady();
        return;
    }
    if (upper == "RESIGN") {
        emit engineResigned();
        return;
    }

    // Coordinate response from genmove
    int x, y;
    if (gtpToCoord(body, x, y)) {
        emit moveReady(x, y);
    } else {
        qWarning() << "[KataGoEngine] Unrecognised response body:" << body;
    }
}

void KataGoEngine::onProcessError(QProcess::ProcessError error)
{
    QString msg;
    switch (error) {
    case QProcess::FailedToStart: msg = "Engine failed to start — check path and permissions"; break;
    case QProcess::Crashed:       msg = "Engine process crashed";                               break;
    case QProcess::Timedout:      msg = "Engine timed out";                                     break;
    default:                      msg = QString("Engine process error (%1)").arg(error);        break;
    }
    qWarning() << "[KataGoEngine]" << msg;
    setState(IDLE);
    emit engineError(msg);
}

void KataGoEngine::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    qDebug() << "[KataGoEngine] Process finished, exit code" << exitCode
             << "status" << status;
    setState(IDLE);
}

void KataGoEngine::onStartupTimeout()
{
    qWarning() << "[KataGoEngine] Startup timed out (60s) — no 'GTP ready' received";
    m_process->kill();
    setState(IDLE);
    emit engineError("Engine startup timed out (60 seconds). "
                     "Check that the engine path is correct and the engine is working.\n"
                     "For SSH engines, verify the host is reachable and key-based auth is configured.");
}

// ---------------------------------------------------------------------------
// Coordinate conversion
// ---------------------------------------------------------------------------

QString KataGoEngine::colorToGtp(StoneColor c) const
{
    return (c == BLACK_STONE) ? "B" : "W";
}

QString KataGoEngine::coordToGtp(int x, int y) const
{
    // x: 0=A, skip I (x>=8 → letter shifts by 1)
    char col = static_cast<char>((x < 8) ? ('A' + x) : ('A' + x + 1));
    int  row = 19 - y;   // y=0 → row 19 (top); y=18 → row 1 (bottom)
    return QString("%1%2").arg(col).arg(row);
}

bool KataGoEngine::gtpToCoord(const QString &vertex, int &x, int &y) const
{
    QString v = vertex.toUpper().trimmed();
    if (v == "PASS")   { x = -1; y = -1; return true; }
    if (v == "RESIGN") { x = -2; y = -2; return true; }
    if (v.length() < 2) return false;

    char col = v[0].toLatin1();
    if (col == 'I') return false;  // 'I' is never valid in GTP

    x = (col >= 'J') ? (col - 'A' - 1) : (col - 'A');

    bool ok;
    int row = v.mid(1).toInt(&ok);
    if (!ok || row < 1 || row > 19) return false;

    y = 19 - row;   // row 19 → y=0; row 1 → y=18
    return true;
}

#include "katago_engine.moc"
