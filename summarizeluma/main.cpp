#include <QtGui>

float measure(const QString &name)
{
    QImage image(name);
    image = image.convertToFormat(QImage::Format_RGB32);
    double sum = 0;
    for (int y=0; y<image.height(); ++y) {
        const unsigned int *p = (const unsigned int *) image.constScanLine(y);
        for (int x=0; x<image.width(); ++x) {
            unsigned int pixel = p[x];
            sum += (((0x00ff0000 & pixel) >> 16)
                    + ((0x0000ff00 & pixel) >> 8)
                    + (0x000000ff & pixel)) / (255 * 3.0);
            // printf("%d,%d: %8x, %f\n", x, y, pixel, level);
        }
    }
    return float(sum / (image.width() * image.height()));
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QStringList names;

    for (int i=1; i<argc; ++i) {
        names.append(argv[i]);
    }

    if (names.size() == 0) {
        printf("no image names specified...\n");
        return 0;
    }

    foreach (const QString &name, names) {
        float level = measure(name);
        printf("%s: %.2f%% 'white'; (%d as unsigned char)\n", qPrintable(name), level * 100, qRound(level * 255));
    }

    return 0;
}
