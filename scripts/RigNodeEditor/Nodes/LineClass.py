"""
LineClass defines all of the capability
needed for representing a connected line between
an input and an output connection between two
different nodes.
Author: Logan Kelly
Date: 3/31/13
"""

from PyQt4.QtGui import *
from PyQt4.QtCore import *
import math

# Line class for connecting the nodes together
class LineClass(QGraphicsLineItem):

    def __init__(self, startItem, endItem, *args, **kwargs):
        super(LineClass, self).__init__(*args, **kwargs)

        # The arrow that's drawn in the center of the line
        self.arrowHead = QPolygonF()
        self.myColor = Qt.black
        self.myStartItem = startItem
        self.myEndItem = endItem
        self.setZValue(-1.0)
        self.setFlags(QGraphicsItem.ItemIsSelectable|QGraphicsItem.ItemIsFocusable)
        self.setPen(QPen(self.myColor, 1, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin))

        '''
        This if statement is making all of the connections consistent. The startItem will always be the
        beginning of the line. The arrow will always point to the end item, no matter which way the user
        connects the line.
        '''
        try:
            if self.myStartItem.isInputConnection:
                temp = self.myStartItem
                self.myStartItem = self.myEndItem
                self.myEndItem = temp
        except AttributeError, e:
            print "Error checking isInputConnection on node %s" %str(e)

    def deleteLine(self):
        # For whatever the shit reason, I have to have this check. If I don't, I get an error in rare cases.
        if self:
            #self.getEndItem().getWidgetMenu().receiveFrom(self.getStartItem(), delete=True)
            #self.getStartItem().getWidgetMenu().sendData(self.getStartItem().getWidgetMenu().packageData())
            #self.getStartItem().removeReferences()
            self.scene().removeItem(self)
            self.myStartItem.connectedLine.remove(self)
            self.myEndItem.connectedLine.remove(self)

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Delete:
            self.deleteLine()
            self.update()
        else:
            super(LineClass, self).keyPressEvent(event)

    def getCenterPoint(self):
        line = self.getLine()
        centerX = (line.p1().x() + line.p2().x())/2
        centerY = (line.p1().y() + line.p2().y())/2
        return QPointF(centerX, centerY)

    def getLine(self):
        p1 = self.myStartItem.sceneBoundingRect().center()
        p2 = self.myEndItem.sceneBoundingRect().center()
        return QLineF(self.mapFromScene(p1), self.mapFromScene(p2))


    def getEndItem(self):
        return self.myEndItem.parentItem()

    def getStartItem(self):
        return self.myStartItem.parentItem()

    def updatePosition(self):
        self.setLine(self.getLine())
        self.myStartItem.connectedLine.append(self)
        self.myEndItem.connectedLine.append(self)

    def boundingRect(self):
        extra = (self.pen().width() + 100)  / 2.0
        line = self.getLine()
        p1 = line.p1()
        p2 = line.p2()
        return QRectF(p1, QSizeF(p2.x() - p1.x(), p2.y() - p1.y())).normalized().adjusted(-extra, -extra, extra, extra)

    def shape(self):
        path = super(LineClass, self).shape()
        path.addPolygon(self.arrowHead)
        return path

    def paint(self, painter, option, widget=None):
        arrowSize = 20.0
        line = self.getLine()
        painter.setBrush(self.myColor)
        myPen = self.pen()
        myPen.setColor(self.myColor)
        painter.setPen(myPen)

        if self.isSelected():
            painter.setBrush(Qt.yellow)
            myPen.setColor(Qt.yellow)
            myPen.setStyle(Qt.DashLine)
            painter.setPen(myPen)


        ####################################
        # This is Calculating the angle between the x-axis and the line of the arrow.
        # Then turning the arrow head to this angle so that it follows the direction of the arrow
        # If the angle is negative, turn the direction of the arrow
        ####################################

        if line.length() > 0.0:

            try:
                angle = math.acos(line.dx() / line.length())
            except ZeroDivisionError:
                angle = 0

            if line.dy() >= 0:
                angle = (math.pi * 2.0) - angle

            # Making sure that no matter which connectionCircle (output or input) is selected first, the arrow always points at the next input connection
            if self.myStartItem.isInputConnection:
                revArrow = 1
            else:
                revArrow = -1

            # Get the center point of the line
            centerPoint = self.getCenterPoint()

            # The head of the arrows tip is the centerPoint, so now calculate the other 2 arrow points
            arrowP1 = centerPoint + QPointF(math.sin(angle + math.pi / 3.0) * arrowSize * revArrow,
                                        math.cos(angle + math.pi / 3) * arrowSize * revArrow)
            arrowP2 = centerPoint + QPointF(math.sin(angle + math.pi - math.pi / 3.0) * arrowSize * revArrow,
                                        math.cos(angle + math.pi - math.pi / 3.0) * arrowSize * revArrow)

            # Clear anything in the arrowHead polygon
            self.arrowHead.clear()

            # Set the points of the arrowHead polygon
            for point in [centerPoint, arrowP1, arrowP2]:
                self.arrowHead.append(point)

            if line:
                painter.drawPolygon(self.arrowHead)
                painter.drawLine(line)