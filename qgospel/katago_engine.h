#ifndef KATAGO_ENGINE_H
#define KATAGO_ENGINE_H

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QQueue>
#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QPair>

#include "game_types.h"

// ---------------------------------------------------------------------------
// KataGoEngine — wraps a GTP engine subprocess (katago or any GTP-compatible
// binary/script) and exposes a Qt signal/slot interface.
//
// Commands are serialised through an internal queue; only one GTP command is
// in-flight at a time.  Responses are parsed on stdout; startup is detected
// on stderr ("GTP ready, beginning main protocol loop").
// ---------------------------------------------------------------------------

class KataGoEngine : public QObject {
    Q_OBJECT

public:
    enum State { IDLE, STARTING, READY, THINKING, PONDERING };

    explicit KataGoEngine(QObject *parent = nullptr);
    ~KataGoEngine();

    // Launch the engine process.  enginePath may be a binary or a shell script.
    // workingDir is set as QProcess working directory (important for relative
    // logDir in default_gtp.cfg).
    void start(const QString &enginePath,
               const QString &engineArgs,
               const QString &workingDir,
               double komi,
               int    handicap,
               int    boardsize     = 19,
               int    timePerMove   = 5);

    // Graceful shutdown: sends "quit", gives process 2 s, then kills it.
    void stop();

    State   state()    const { return m_state; }
    bool    isReady()  const { return m_state == READY || m_state == PONDERING; }
    bool    isOwnershipInFlight() const { return m_awaiting_ownership; }

public slots:
    void sendPlay(StoneColor color, int x, int y);
    void sendPass(StoneColor color);
    void requestGenmove(StoneColor color);
    void requestFinalScore();
    void requestOwnership();  // sends kata-raw-nn 0; emits ownershipReady on ACK
    void cancelOwnership();   // cancel an in-flight ownership request (discard response)
    void clearBoard();                     // resets position; emits boardCleared() on ACK
    void enqueueRaw(const QString &cmd);  // manual console passthrough

signals:
    void engineReady();
    void boardCleared();                   // clear_board ACK received — ready for new game
    void moveReady(int x, int y);
    void passMoveReady();
    void engineResigned();
    void handicapStonesReady(QList<QPair<int,int>> stones);
    void scoreReady(const QString &result);
    void ownershipReady(QVector<float> ownership); // 361 floats, row-major top→bottom left→right
    void engineError(const QString &msg);
    void illegalMove(const QString &gtpVertex);          // play command rejected; queue flushed
    void gtpLogLine(const QString &line, bool is_sent);  // for engine console display

private slots:
    void onStderrReady();
    void onStdoutReady();
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onStartupTimeout();

private:
    void setState(State s);
    void enqueue(const QString &cmd);
    void dispatchNext();
    void handleResponse(const QString &body);
    void onStderrReady_handleGTPReady();  // shared init sequence trigger

    // Coordinate conversion
    QString colorToGtp(StoneColor c) const;
    QString coordToGtp(int x, int y) const;
    bool    gtpToCoord(const QString &vertex, int &x, int &y) const;

    QProcess        *m_process;
    State            m_state;
    QQueue<QString>  m_cmd_queue;
    QString          m_pending_cmd;   // command currently in-flight
    QTimer          *m_startup_timer; // 30-second watchdog
    QString          m_stdout_buf;    // partial-line accumulator

    double  m_komi;
    int     m_handicap;
    int     m_boardsize;
    int     m_time_per_move;
    int     m_init_acks_remaining;    // countdown for time_settings/boardsize/komi/clear_board ACKs
    bool    m_clear_board_pending;    // true when a user-initiated clear_board is in-flight
    bool    m_stop_for_score;         // true when stop was sent to interrupt genmove before final_score
    bool    m_awaiting_ownership;     // true while kata-raw-nn response is in-flight
    QVector<float>  m_latest_ownership;  // ownership values from kata-raw-nn response
};

#endif // KATAGO_ENGINE_H
