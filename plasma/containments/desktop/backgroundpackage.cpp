#include "backgroundpackage.h"
#include <cmath>
// <cmath> does not define fabs (by the standard, even if it does with gcc)
#include <math.h>

#include <QFileInfo>
#include <QPainter>
#include <KDebug>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KSvgRenderer>
#include <plasma/packagestructure.h>
#include <plasma/packagemetadata.h>
#include <ThreadWeaver/Weaver>

using namespace Plasma;

class ResizeThread : public ThreadWeaver::Job
{
public:
    ResizeThread(const QString &path, float ratio, QObject *parent = 0);
    virtual ~ResizeThread();
    
    virtual void start(QPersistentModelIndex index);
    virtual void run();
    
    QImage result() const;
    QPersistentModelIndex index() const;
    bool isInitialized() const;
private:
    QString m_path;
    QImage m_result;
    float m_ratio;
    QPersistentModelIndex m_index;
};

ResizeThread::ResizeThread(const QString &path, float ratio, QObject *parent)
: ThreadWeaver::Job(parent)
, m_path(path)
, m_ratio(ratio)
{
}

ResizeThread::~ResizeThread() {
}

void ResizeThread::start(QPersistentModelIndex index)
{
    m_index = index;
    ThreadWeaver::Weaver::instance()->enqueue(this);
}

bool ResizeThread::isInitialized() const
{
    return m_index.isValid();
}

void ResizeThread::run()
{
    m_result = Background::createScreenshot(m_path, m_ratio);
}

QImage ResizeThread::result() const
{
    if (isFinished()) {
        return m_result;
    }
    else {
        return QImage();
    }
}

QPersistentModelIndex ResizeThread::index() const
{
    return m_index;
}

Background::~Background()
{
}

QImage Background::createScreenshot(const QString &path, float ratio)
{
    if (path.endsWith("svg") || path.endsWith("svgz")) {
        KSvgRenderer renderer(path);
        QImage img(QSize(int(SCREENSHOT_HEIGHT * ratio), SCREENSHOT_HEIGHT),
                   QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter p(&img);
        renderer.render(&p);
        return img;
    }
    else {
        QImage img(path);
        if (!img.isNull()) {
            return img.scaled(int(SCREENSHOT_HEIGHT * ratio), 
                            SCREENSHOT_HEIGHT,
                            Qt::KeepAspectRatio);
        }
        else {
            return defaultScreenshot();
        }
    }

}

QImage Background::defaultScreenshot()
{
    static QImage defaultScreenshotImage;
    
    if (defaultScreenshotImage.isNull()) {
        QImage img(QSize(SCREENSHOT_HEIGHT, SCREENSHOT_HEIGHT), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter p(&img);
        p.drawText(QRect(0, 0, SCREENSHOT_HEIGHT, SCREENSHOT_HEIGHT), 
                   Qt::AlignHCenter | Qt::AlignVCenter,
                   "Preview\nnot\navailable");
        defaultScreenshotImage = img;
    }
    return defaultScreenshotImage;
}


class BackgroundPackageStructure : public PackageStructure
{
public:
    static const BackgroundPackageStructure &self();
private:
    BackgroundPackageStructure(); // should be used as a singleton
    void addResolution(const char *res);
};

BackgroundPackageStructure::BackgroundPackageStructure()
: PackageStructure("Background")
{
    QStringList mimetypes;
    mimetypes << "image/svg" << "image/png" << "image/jpeg" << "image/jpg";
    setDefaultMimetypes(mimetypes);

    addDirectoryDefinition("images", "images", i18n("Images"));
    addFileDefinition("screenshot", "screenshot.png", i18n("Screenshot"));
}


const BackgroundPackageStructure &BackgroundPackageStructure::self()
{
    static BackgroundPackageStructure instance;
    return instance;
}



BackgroundPackage::BackgroundPackage(const QString &path, float ratio)
: Package(path, BackgroundPackageStructure::self())
, m_path(path)
, m_ratio(ratio)
{
}

QString BackgroundPackage::resString(const QSize &size) const
{
    return QString::number(size.width()) + 'x' + QString::number(size.height());
}

QSize BackgroundPackage::resSize(const QString &str) const
{
    int index = str.indexOf('x');
    if (index != -1) {
        return QSize(str.left(index).toInt(), 
                     str.mid(index + 1).toInt());
    }
    else {
        return QSize();
    }
}

QString BackgroundPackage::findBackground(const QSize &size, 
                                          ResizeMethod method) const
{
    QStringList images = entryList("images");
    if (images.empty()) {
        return QString();
    }
    
    // choose the nearest resolution
    float best;
    QString bestImage;
    foreach (QString entry, images) {
        QSize candidate = resSize(QFileInfo(entry).baseName());
        if (candidate == QSize()) {
            continue;
        }
        
        double dist = distance(candidate, size, method);
        kDebug() << "candidate" << candidate << "distance" << dist;
        if (bestImage.isNull() || dist < best) {
            bestImage = filePath("images", entry);
            best = dist;
            kDebug() << "best" << bestImage;
        }
    }
    
    kDebug() << "best image" << bestImage;
    return bestImage;
}

float BackgroundPackage::distance(const QSize& size,
                                   const QSize& desired,
                                   ResizeMethod method) const
{
    // compute difference of areas
    float delta = size.width() * size.height() -
                  desired.width() * desired.height();
    // scale down to about 1.0
    delta /= 1000000.0;
    
    switch (method) {
    case Scale: {
        // Consider first the difference in aspect ratio,
        // then in areas. Prefer scaling down.
        float deltaRatio = size.width() / size.height() -
                           desired.width() / desired.height();
        return fabs(deltaRatio) * 3.0 + 
            (delta >= 0.0 ? delta : -delta + 5.0);
    }
    case ScaleCrop:
        // Difference of areas, slight preference to scale down
        return delta >= 0.0 ? delta : -delta + 2.0;
    case Center:
    default:
        // Difference in areas
        return fabs(delta);
    }
}

QPixmap BackgroundPackage::screenshot() const
{
    if (m_screenshot.isNull()) {
        QString screenshotPath = filePath("screenshot");
        if (!screenshotPath.isEmpty()) {
            QImage img = createScreenshot(screenshotPath, m_ratio);
            m_screenshot = QPixmap::fromImage(img);
        }
    }
    
    return m_screenshot;
}

bool BackgroundPackage::screenshotGenerationStarted() const
{
    return true;
}

void BackgroundPackage::generateScreenshot(QPersistentModelIndex) const
{
}

QString BackgroundPackage::title() const
{
    return metadata()->name();
}

QString BackgroundPackage::author() const
{
    return metadata()->author();
}

QString BackgroundPackage::email() const
{
    return metadata()->email();
}

QString BackgroundPackage::license() const
{
    return metadata()->license();
}

bool BackgroundPackage::isValid() const
{
    return Package::isValid();
}

QString BackgroundPackage::path() const
{
    return m_path;
}


BackgroundFile::BackgroundFile(const QString &file, float ratio)
: m_file(file)
, m_ratio(ratio)
, m_resizer_started(false)
{
}

BackgroundFile::~BackgroundFile()
{
}

QString BackgroundFile::findBackground(const QSize &,
                                       ResizeMethod) const
{
    return m_file;
}

QPixmap BackgroundFile::screenshot() const
{
    return m_screenshot;
}

bool BackgroundFile::screenshotGenerationStarted() const
{
    return m_resizer_started;
}

void BackgroundFile::generateScreenshot(QPersistentModelIndex index) const
{
    ResizeThread *resizer = new ResizeThread(m_file, m_ratio);
    connect(resizer, SIGNAL(done(ThreadWeaver::Job *)),
            this, SLOT(updateScreenshot(ThreadWeaver::Job *)));
    m_resizer_started = true;
    resizer->start(index);
}

void BackgroundFile::updateScreenshot(ThreadWeaver::Job *job)
{
    ResizeThread *resizer = static_cast<ResizeThread *>(job);
    m_screenshot = QPixmap::fromImage(resizer->result());
    emit screenshotDone(resizer->index());
    resizer->deleteLater();
}

QString BackgroundFile::author() const
{
    return QString();
}

QString BackgroundFile::title() const
{
    return QFileInfo(m_file).baseName();
}

QString BackgroundFile::email() const
{
    return QString();
}

QString BackgroundFile::license() const
{
    return QString();
}

bool BackgroundFile::isValid() const
{
    return true;
}

QString BackgroundFile::path() const
{
    return m_file;
}
