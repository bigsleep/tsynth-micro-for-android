package org.bigsleep.android.view

import android.util.Log
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.{GL10, GL11}

import org.bigsleep.geometry._

case class CircleDrawer(circle : Circle, brush : Brush = Brush()) extends Drawer
{
    val vertexSize = 65
    val vertex = initVertexArray
    
    private def initVertexArray =
    {
        val arr = Array.fill[Int](vertexSize * 2)(0)
        val c = Vec2(circle.center.x.toDouble, circle.center.y.toDouble)
        val r = circle.radius.toDouble
        val R = Mat2.rotationM(math.Pi * 2d / (vertexSize - 1).toDouble)
        
        var n = 0
        var p = Vec2(0d, r)
        while(n < vertexSize){
            arr(n * 2) = toFixed(p.x + c.x)
            arr(n * 2 + 1) = toFixed(p.y + c.y)
            p = R * p
            n += 1
        }
        getIntBuffer(arr)
    }
    
    def apply(gl : GL10)
    {
        gl.glDisable(GL10.GL_TEXTURE_2D)
        gl.glEnableClientState(GL10.GL_VERTEX_ARRAY)
        brush.fill match
        {
            case Some(SolidColorFill(color)) =>
            {
                gl.glColor4x(toFixed(color.r), toFixed(color.g), toFixed(color.b), toFixed(color.a))
                gl.glHint(GL10.GL_LINE_SMOOTH_HINT, GL10.GL_NICEST)
                gl.glVertexPointer(2, GL10.GL_FIXED, 0, vertex)
                gl.glDrawArrays(GL10.GL_TRIANGLE_FAN, 0, vertexSize)
            }
            case _ => {}
        }
        brush.stroke match
        {
            case Some(SolidColorStroke(width, color)) =>
            {
                gl.glColor4x(toFixed(color.r), toFixed(color.g), toFixed(color.b), toFixed(color.a))
                gl.glHint(GL10.GL_LINE_SMOOTH_HINT, GL10.GL_NICEST)
                gl.glLineWidthx(toFixed(width))
                gl.glVertexPointer(2, GL10.GL_FIXED, 0, vertex)
                gl.glDrawArrays(GL10.GL_LINE_LOOP, 0, vertexSize)
            }
            case _ => {}
        }
        gl.glDisableClientState(GL10.GL_VERTEX_ARRAY)
    }
}


case class RectangleDrawer(rectangle : Rectangle, brush : Brush = Brush()) extends Drawer
{
    val vertexSize = 4
    val vertex = initVertexArray
    
    private def initVertexArray =
    {
        val ll = Vec2(rectangle.leftLower.x.toDouble, rectangle.leftLower.y.toDouble)
        val w = rectangle.width.toDouble
        val h = rectangle.height.toDouble
        val arr = Array(
            toFixed(ll.x), toFixed(ll.y),
            toFixed(ll.x), toFixed(ll.y + h),
            toFixed(ll.x + w), toFixed(ll.y + h),
            toFixed(ll.x + w), toFixed(ll.y))
        getIntBuffer(arr)
    }
    
    def apply(gl : GL10)
    {
        gl.glDisable(GL10.GL_TEXTURE_2D)
        gl.glEnableClientState(GL10.GL_VERTEX_ARRAY)
        brush.fill match
        {
            case Some(SolidColorFill(color)) =>
            {
                gl.glColor4x(toFixed(color.r), toFixed(color.g), toFixed(color.b), toFixed(color.a))
                gl.glHint(GL10.GL_LINE_SMOOTH_HINT, GL10.GL_NICEST)
                gl.glVertexPointer(2, GL10.GL_FIXED, 0, vertex)
                gl.glDrawArrays(GL10.GL_TRIANGLE_FAN, 0, vertexSize)
            }
            case _ => {}
        }
        brush.stroke match
        {
            case Some(SolidColorStroke(width, color)) =>
            {
                gl.glColor4x(toFixed(color.r), toFixed(color.g), toFixed(color.b), toFixed(color.a))
                gl.glHint(GL10.GL_LINE_SMOOTH_HINT, GL10.GL_NICEST)
                gl.glLineWidthx(toFixed(width))
                gl.glVertexPointer(2, GL10.GL_FIXED, 0, vertex)
                gl.glDrawArrays(GL10.GL_LINE_LOOP, 0, vertexSize)
            }
            case _ => {}
        }
        gl.glDisableClientState(GL10.GL_VERTEX_ARRAY)
    }
}

case class PolyLineDrawer(polyline : PolyLine, brush : Brush = Brush()) extends Drawer
{
    val vertexSize = polyline.points.size
    val vertex = initVertexArray
    
    private def initVertexArray =
    {
        val arr = Array.fill(vertexSize * 2)(0)
        (0 to vertexSize - 1).foreach{i =>
            arr(i * 2) = toFixed(polyline.points(i).x.toDouble)
            arr(i * 2 + 1) = toFixed(polyline.points(i).y.toDouble)
        }
        getIntBuffer(arr)
    }
    
    def apply(gl : GL10)
    {
        gl.glDisable(GL10.GL_TEXTURE_2D)
        gl.glEnableClientState(GL10.GL_VERTEX_ARRAY)
        brush.stroke match
        {
            case Some(SolidColorStroke(width, color)) =>
            {
                gl.glColor4x(toFixed(color.r), toFixed(color.g), toFixed(color.b), toFixed(color.a))
                gl.glHint(GL10.GL_LINE_SMOOTH_HINT, GL10.GL_NICEST)
                gl.glLineWidthx(toFixed(width))
                gl.glVertexPointer(2, GL10.GL_FIXED, 0, vertex)
                gl.glDrawArrays(GL10.GL_LINE_STRIP, 0, vertexSize)
            }
            case _ => {}
        }
        gl.glDisableClientState(GL10.GL_VERTEX_ARRAY)
    }
}

object ShapeDrawer
{
    def apply(shape : Shape, brush : Brush) : Drawer =
    shape match
    {
        case x : Circle => CircleDrawer(x, brush)
        case x : Rectangle => RectangleDrawer(x, brush)
        case x : PolyLine => PolyLineDrawer(x, brush)
    }
}

case class Color(r : Double, g : Double, b : Double, a : Double)
object Color
{
    val BLACK = Color(0d, 0d, 0d, 1d)
    val WHITE = Color(1d, 1d, 1d, 1d)
    val RED = Color(1d, 0d, 0d, 1d)
}

abstract class Stroke
case class SolidColorStroke(width : Double = 1d, color : Color = Color.BLACK) extends Stroke

abstract class Fill
case class SolidColorFill(color : Color = Color.BLACK) extends Fill

case class Brush(stroke : Option[Stroke] = Some(SolidColorStroke()), fill : Option[Fill] = None)

object Brush
{
    def apply(s : Stroke, f : Fill) : Brush = Brush(Some(s), Some(f))
}


