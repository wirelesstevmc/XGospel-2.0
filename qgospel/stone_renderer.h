#ifndef STONE_RENDERER_H
#define STONE_RENDERER_H

#include <QPixmap>
#include <QImage>
#include <QColor>
#include <QList>
#include <cmath>

// Stone rendering based on q5Go's imagehandler
// Implements "Goban" style stones with 3D shading and white stone grain patterns
class StoneRenderer {
public:
    StoneRenderer();

    // Generate stone pixmaps for a given stone size
    void generateStones(int stone_diameter);

    // Get the black stone pixmap
    const QPixmap& getBlackStone() const { return black_stone; }

    // Get a white stone pixmap (randomly varied for natural appearance)
    const QPixmap& getWhiteStone(int variation_index) const;

    // Get shadow pixmap
    const QPixmap& getShadow() const { return shadow_stone; }

private:
    struct WhiteDesc {
        double cosTheta, sinTheta;
        double stripeWidth, xAdd;
        double stripeMul, zMul;
    };

    // Pre-generated random values for white stone grain patterns
    double m_pregen_rnd[300];

    // Pixmaps
    QPixmap black_stone;
    QList<QPixmap> white_stones;  // Multiple variations for natural look
    QPixmap shadow_stone;

    // Current stone size
    int m_stone_size;

    // Shadow intensity (0-1)
    double m_shadow;

    // Rendering functions
    void decideAppearance(WhiteDesc *desc, int size, int rnd_idx);
    double getStripe(WhiteDesc &white, double bright, double z, int x, int y, double range);
    double shade_point(double x, double y, double material, double ratio, double hardness, double ambient_ratio);
    void render(unsigned *dest, double *intense, int d, const QColor &in_col);

    // Paint individual stone types
    void paint_stone_new(QImage &img, int d, const QColor &col, double hard, double spec,
                        int flat, double radius, bool clamshell, int idx);
    void paint_shadow_stone(QImage &img, int d);

    // Helper to copy image data
    void icopy(unsigned *im, QImage &qim, int w, int h);
};

#endif // STONE_RENDERER_H
