#ifndef USERARROW_H
#define USERARROW_H
#include "auxilary.h"

class UserArrow
        :public QGraphicsPathItem
{
public:
    explicit UserArrow(QGraphicsItem *parent = 0);
    explicit UserArrow(QPointF end, QGraphicsItem *parent = 0);

    void setEndpoint(QPointF end);
    void setEndpoint(qreal x, qreal y);
    QPointF Endpoint() const;

private:
    QPointF end = {0,0};
    qreal endPointRadius = 10;
};

using pUserArrow = std::shared_ptr<UserArrow>;


#endif // USERARROW_H
