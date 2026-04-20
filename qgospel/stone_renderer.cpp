#include "stone_renderer.h"
#include <QPainter>
#include <QDebug>
#include <cstdlib>
#include <ctime>

// Random number generator for Windows compatibility
#ifdef Q_OS_WIN
static double drand48() { return rand() * 1.0 / RAND_MAX; }
#endif

StoneRenderer::StoneRenderer()
    : m_stone_size(0), m_shadow(0.9)
{
    // Pre-generate random values for white stone grain patterns
    // This ensures consistent appearance across stone renderings
    srand(42);  // Fixed seed for reproducibility
    for (int i = 0; i < 300; i++) {
        m_pregen_rnd[i] = drand48();
    }
}

void StoneRenderer::generateStones(int stone_diameter) {
    m_stone_size = stone_diameter;

    // Generate black stone (solid with 3D shading)
    // Use q5Go preset 0 (Goban) parameters
    // radius=95 -> 2.05 + (100-95)/30 = 2.217, hard=16 -> 1+16/10=2.6, spec=38/100=0.38, flat=1
    QImage black_img(stone_diameter, stone_diameter, QImage::Format_ARGB32);
    black_img.fill(Qt::transparent);
    paint_stone_new(black_img, stone_diameter, QColor(60, 60, 60), 2.6, 0.38, 1, 2.217, false, 0);
    black_stone = QPixmap::fromImage(black_img);

    // Generate multiple white stone variations (with grain patterns)
    // Use q5Go preset 0 (Goban) parameters
    // radius=95 -> 2.217, hard=0 -> 1+0/10=1.0, spec=61/100=0.61, flat=1, clamshell=true
    white_stones.clear();
    for (int i = 0; i < 10; i++) {  // 10 variations for variety
        QImage white_img(stone_diameter, stone_diameter, QImage::Format_ARGB32);
        white_img.fill(Qt::transparent);
        paint_stone_new(white_img, stone_diameter, QColor(255, 255, 255), 1.0, 0.61, 1, 2.217, true, i);
        white_stones.append(QPixmap::fromImage(white_img));
    }

    // Generate shadow
    QImage shadow_img(stone_diameter, stone_diameter, QImage::Format_ARGB32);
    shadow_img.fill(Qt::transparent);
    paint_shadow_stone(shadow_img, stone_diameter);
    shadow_stone = QPixmap::fromImage(shadow_img);
}

const QPixmap& StoneRenderer::getWhiteStone(int variation_index) const {
    if (white_stones.isEmpty()) {
        // Return black stone as fallback if white stones not generated yet
        return black_stone;
    }
    return white_stones[variation_index % white_stones.count()];
}

void StoneRenderer::decideAppearance(WhiteDesc *desc, int size, int rnd_idx) {
    double minStripeW, maxStripeW, theta;

    minStripeW = (double)size / 20.0;
    if (minStripeW < 1.5)
        minStripeW = 1.5;
    maxStripeW = (double)size / 10.0;
    if (maxStripeW < 2.5)
        maxStripeW = 2.5;

    theta = m_pregen_rnd[rnd_idx] * 2.0 * M_PI;
    desc->cosTheta = cos(theta);
    desc->sinTheta = sin(theta);
    desc->stripeWidth = 1.5 * minStripeW + (m_pregen_rnd[rnd_idx + 1] * (maxStripeW - minStripeW));

    desc->xAdd = 3 * desc->stripeWidth + (double)size * 3.0;

    desc->stripeMul = 3.0;
    desc->zMul = m_pregen_rnd[rnd_idx + 2] * 650.0 + 70.0;
}

double StoneRenderer::getStripe(WhiteDesc &white, double bright, double z, int x, int y, double range) {
    double wBright;

    double wStripeLoc = x * white.cosTheta - y * white.sinTheta + white.xAdd;
    double wStripeColor = fmod(wStripeLoc + (z * z * z * white.zMul) * white.stripeWidth,
                              white.stripeWidth) / white.stripeWidth;
    wStripeColor = wStripeColor * white.stripeMul - 0.5;
    if (wStripeColor < 0.0)
        wStripeColor = -2.0 * wStripeColor;
    if (wStripeColor > 1.0)
        wStripeColor = 1.0;
    wStripeColor = wStripeColor * range + (1 - range);

    // q5Go always uses the first branch (has "1 ||" in condition)
    wBright = bright * wStripeColor;

    if (wBright > 1)
        wBright = 1;
    if (wBright < 0)
        wBright = 0;

    return wBright;
}

void StoneRenderer::paint_shadow_stone(QImage &si, int d) {
    unsigned *pw = new unsigned[d * d];
    int i, j, k;
    double di, dj, d2 = (double)d / 2.0 - 5e-1, r = d2 - 2e-1;
    double hh;

    k = 0;

    for (i = 0; i < d; i++)
        for (j = 0; j < d; j++) {
            di = i - d2;
            dj = j - d2;
            hh = r - sqrt(di * di + dj * dj);
            if (hh >= 0) {
                hh = 2 * hh / r;
                hh *= m_shadow;
                if (hh > 1) hh = 1;

                // Very dark shadow (10,8,6) - matches q5Go's strong shadow effect
                int shadow_r = 10;
                int shadow_g = 8;
                int shadow_b = 6;
                pw[k] = ((int)(255 * hh) << 24) | (shadow_r << 16) | (shadow_g << 8) | shadow_b;
            }
            else
                pw[k] = 0;
            k++;
        }

    icopy(pw, si, d, d);
    delete[] pw;
}

double StoneRenderer::shade_point(double x, double y, double material, double ratio, double hardness, double ambient_ratio) {
    double light_x = -0.4;
    double light_y = 0.4;
    double light_len_xy = sqrt(light_x * light_x + light_y * light_y);
    double light_z = sqrt(1 - light_len_xy * light_len_xy);
    double center_dist = sqrt(x * x + y * y);
    double z = sqrt(1 - center_dist * center_dist);

    double dotprod = x * light_x + y * light_y + z * light_z;
    double reflect_z = 2 * dotprod * z - light_z;
    double spdot = reflect_z;
    double specular = pow(spdot, hardness);
    double diffuse = dotprod * material;
    double s_ratio = (1 - ambient_ratio) * ratio;
    double d_ratio = (1 - ambient_ratio) * (1 - ratio);
    double ambient = ambient_ratio * material;
    double intensity = ambient + specular * s_ratio + diffuse * d_ratio;

    return intensity;
}

void StoneRenderer::render(unsigned *dest, double *intense, int d, const QColor &in_col) {
    double pix_width = 2.0 / d;
    double pic_radius = 0.97;

    // Get base color's HSV - we'll use H and S, but intensity provides V
    int base_h, base_s, base_v;
    in_col.getHsv(&base_h, &base_s, &base_v);

    for (int k = 0; k < d * d; k++) {
        int i = k % d;
        int j = k / d;
        double norm_x = 2.0 * i / d - 1;
        double norm_y = 2.0 * j / d - 1;

        double dist = sqrt(norm_x * norm_x + norm_y * norm_y);
        double alpha = 1;
        if (dist >= pic_radius - pix_width) {
            dist -= pic_radius - pix_width;
            if (dist < pix_width)
                alpha = 1 - (dist / pix_width);
            else
                alpha = 0;
        }

        // Create color from intensity - keep base H and S, use intensity for V
        // Don't clamp - let intensity > 1.0 (from specular highlights) saturate to 255
        // Qt's setHsv will clamp values > 255 to 255 automatically
        QColor col;
        col.setHsv(base_h, base_s, (int)(255 * intense[k]));
        int r = 0, g = 0, b = 0;
        if (alpha > 0)
            col.getRgb(&r, &g, &b);
        dest[k] = (unsigned)(alpha * 255) * 0x01000000 + r * 0x010000 + g * 0x0100 + b;
    }
}

void StoneRenderer::paint_stone_new(QImage &wi, int d, const QColor &col, double hard, double spec,
                                    int flat, double radius, bool clamshell, int idx) {
    WhiteDesc desc;
    decideAppearance(&desc, d, idx * 3);
    double f = sqrt(3);

    double d2 = (double)d / 2.0 - 5e-1;
    double r = d2 - 2e-1;
    double h, s, v;
    col.getHsvF(&h, &s, &v);

    double *intense = new double[d * d];

    double max_int = 0;

    int k = 0;
    for (int i = 0; i < d; i++)
        for (int j = 0; j < d; j++) {
            double norm_x = 2.0 * i / d - 1;
            double norm_y = 2.0 * j / d - 1;

            double dist = sqrt(norm_x * norm_x + norm_y * norm_y);
            if (dist <= 1) {
                double newdist = dist;
                if (dist != 0) {
                    double rdist = sqrt(dist);
                    if (flat > 0)
                        newdist *= rdist;
                    if (flat > 1)
                        newdist *= rdist;
                    if (flat > 2)
                        newdist *= rdist;
                    if (flat > 3)
                        newdist *= rdist;
                    if (flat > 4)
                        newdist *= rdist;
                    norm_x = norm_x * (newdist / dist);
                    norm_y = norm_y * (newdist / dist);
                }

                double v2 = v;

                if (clamshell) {
                    double x = norm_x * d / 2;
                    double y = norm_y * d / 2;
                    double z = r * r - x * x - y * y;
                    if (z > 0)
                        z = sqrt(z) * f;
                    else
                        z = 0;

                    // q5Go's clamshell grain calculation
                    double xr = sqrt(6 * (x * x + y * y + z * z));
                    double xr1 = (2 * z - x + y) / xr;
                    v2 = getStripe(desc, v, xr1 / 7.0, i, j, 0.15);
                }

                // Divide by radius for shading calculation (q5Go line 255-256)
                norm_x /= radius;
                norm_y /= radius;

                // Use q5Go's ambient value (preset 0 Goban = 15/100 = 0.15)
                intense[k] = shade_point(norm_x, norm_y, v2, spec, hard, 0.15);
                if (intense[k] > max_int)
                    max_int = intense[k];
            } else {
                intense[k] = 0;
            }
            k++;
        }

    // Do NOT normalize intensity - q5Go doesn't do this
    // The material value (v2) in shade_point already differentiates black vs white
    // Normalization would destroy this difference

    unsigned *pw = new unsigned[d * d];
    render(pw, intense, d, col);
    icopy(pw, wi, d, d);

    delete[] pw;
    delete[] intense;
}

void StoneRenderer::icopy(unsigned *im, QImage &qim, int w, int h) {
    for (int y = 0; y < h; y++) {
        uint *p = (uint *)qim.scanLine(y);
        for (int x = 0; x < w; x++) {
            p[x] = im[y * h + x];
        }
    }
}
