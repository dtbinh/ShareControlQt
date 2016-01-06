#ifndef GRAPHICSROBOTINFOITEM_H
#define GRAPHICSROBOTINFOITEM_H
#pragma warning(disable: 4819 4100)

#include <QGraphicsSimpleTextItem>
#include <memory>

class GraphicsRobotItem;
class GraphicsRobotInfoItem : public QGraphicsSimpleTextItem{
public:
    explicit GraphicsRobotInfoItem(GraphicsRobotItem* parent);

    void update();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};
using pGraphicsRobotInfoItem = std::shared_ptr<GraphicsRobotInfoItem>;


#endif // GRAPHICSROBOTINFOITEM_H
