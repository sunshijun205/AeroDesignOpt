#include "airfoilstore.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

AirfoilStore::AirfoilStore()
{
    m_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (m_dir.isEmpty())
        m_dir = QDir::homePath() + QStringLiteral("/.aerodesignopt");
}

QString AirfoilStore::filePath() const
{
    return m_dir + QStringLiteral("/airfoil_analysis.json");
}

bool AirfoilStore::save(const AirfoilInput &input, QString *errorMessage)
{
    QDir dir(m_dir);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("保存失败：无法创建目录 %1").arg(m_dir);
        return false;
    }

    QJsonObject obj;
    obj[QStringLiteral("naca")] = input.naca;
    obj[QStringLiteral("alphaDeg")] = input.alphaDeg;
    obj[QStringLiteral("reynolds")] = input.reynolds;
    obj[QStringLiteral("mach")] = input.mach;
    obj[QStringLiteral("aspectRatio")] = input.aspectRatio;

    QFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("保存失败：无法写入 %1（%2）")
                                .arg(filePath(), file.errorString());
        return false;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

AirfoilInput AirfoilStore::load(bool *ok)
{
    AirfoilInput input;
    QFile file(filePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        if (ok)
            *ok = false;
        return input;
    }
    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        if (ok)
            *ok = false;
        return input;
    }
    const QJsonObject obj = doc.object();
    if (obj.contains(QStringLiteral("naca"))) {
        input.naca = obj[QStringLiteral("naca")].toString(input.naca);
        input.nacaKnown = true;
    }
    if (obj.contains(QStringLiteral("alphaDeg"))) {
        input.alphaDeg = obj[QStringLiteral("alphaDeg")].toDouble(input.alphaDeg);
        input.alphaKnown = true;
    }
    if (obj.contains(QStringLiteral("reynolds"))) {
        input.reynolds = obj[QStringLiteral("reynolds")].toDouble(input.reynolds);
        input.reynoldsKnown = true;
    }
    if (obj.contains(QStringLiteral("mach"))) {
        input.mach = obj[QStringLiteral("mach")].toDouble(input.mach);
        input.machKnown = true;
    }
    if (obj.contains(QStringLiteral("aspectRatio"))) {
        input.aspectRatio = obj[QStringLiteral("aspectRatio")].toDouble(input.aspectRatio);
        input.aspectRatioKnown = true;
    }
    if (ok)
        *ok = true;
    return input;
}
