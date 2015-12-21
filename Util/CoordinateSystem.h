#ifndef COORDINATESYSTEM_H
#define COORDINATESYSTEM_H
#pragma warning(disable: 4819 4100)

#include <QPointF>
#include <QString>
#include <memory>
#include <QTransform>
#include <QObject>

enum class AxisHeading{
    left, right, up, down
};

class CoordinateSystem : public QObject, public QTransform{
    Q_OBJECT
public:
    CoordinateSystem();
    CoordinateSystem(qreal unit, AxisHeading x = AxisHeading::right, AxisHeading y = AxisHeading::up);
    bool validAxisHeading();
    bool buildSystem();

    qreal UnitSize() const;
    AxisHeading XHeading() const;
    AxisHeading YHeading() const;

signals:
    void prepareCoordinateSystemChange(CoordinateSystem* newSystem);

public slots:
    void setUnitSize(qreal newUnitSize);
    void setXHeading(AxisHeading newHeading);
    void setYHeading(AxisHeading newHeading);
    void setAxisHedaing(AxisHeading xNewHeading, AxisHeading yNewHeading);

protected:
    AxisHeading xAxis = AxisHeading::right;
    AxisHeading yAxis = AxisHeading::up;
    qreal unitSize = 100;
};
using pCoordinateSystem = std::shared_ptr<CoordinateSystem>;

inline CoordinateSystem::CoordinateSystem(){
    buildSystem();
}
inline CoordinateSystem::CoordinateSystem(qreal unit, AxisHeading x, AxisHeading y){
    xAxis = x; yAxis = y; unitSize = unit;
    buildSystem();
}

inline bool CoordinateSystem::validAxisHeading(){
    if (xAxis == AxisHeading::left || xAxis == AxisHeading::right){
        if (yAxis == AxisHeading::left || yAxis == AxisHeading::right)
            return false;
    }
    if (xAxis == AxisHeading::up || xAxis == AxisHeading::down){
        if (yAxis == AxisHeading::up || yAxis == AxisHeading::down)
            return false;
    }
    return true;
}
inline bool CoordinateSystem::buildSystem(){
    if (!validAxisHeading())
        return false;

    qreal m11, m12, m13, m21, m22, m23, m31, m32, m33;
    m11 = m12 = m13 = m21 = m22 = m23 = m31 = m32 = m33 = 0;

    m33 = 1;
    switch (xAxis){
    case AxisHeading::left: m11 = -1; break;    // m11 : x2 = m11 * x1
    case AxisHeading::right:m11 = 1;  break;
    case AxisHeading::up:   m12 = -1;break;      // m12 : y2 = m12 * x1
    case AxisHeading::down: m12 = 1;break;
    }
    switch (yAxis){
    case AxisHeading::left: m21 = -1; break;    // m21 : x2 = m21 * y1
    case AxisHeading::right:m21 = 1;  break;
    case AxisHeading::up:   m22 = -1;break;     // m22 : y2 = m22 * y1
    case AxisHeading::down: m22 = 1;break;
    }
    QTransform* t = this;
    t->setMatrix(m11, m12, m13,
                 m21, m22, m23,
                 m31, m32, m33);
    return true;
}
inline void CoordinateSystem::setUnitSize(qreal newUnitSize){
    if (unitSize != newUnitSize){
        emit prepareCoordinateSystemChange(this);
        unitSize = newUnitSize;
        buildSystem();
    }
}
inline void CoordinateSystem::setXHeading(AxisHeading newHeading){
    if (xAxis != newHeading){
        emit prepareCoordinateSystemChange(this);
        xAxis = newHeading;
        buildSystem();
    }
}
inline void CoordinateSystem::setYHeading(AxisHeading newHeading){
    if (yAxis != newHeading){
        emit prepareCoordinateSystemChange(this);
        yAxis = newHeading;
        buildSystem();
    }
}
inline void CoordinateSystem::setAxisHedaing(AxisHeading xNewHeading, AxisHeading yNewHeading){
    if (xAxis != xNewHeading || yAxis != yNewHeading){
        emit prepareCoordinateSystemChange(this);
        xAxis = xNewHeading;
        yAxis = yNewHeading;
        buildSystem();
    }
}
inline qreal CoordinateSystem::UnitSize() const{
    return unitSize;
}
inline AxisHeading CoordinateSystem::XHeading() const{
    return xAxis;
}
inline AxisHeading CoordinateSystem::YHeading() const{
    return yAxis;
}

#endif // COORDINATESYSTEM_H
