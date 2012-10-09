package org.bigsleep.android.view

import android.util.Log
import android.graphics.{Bitmap, Paint, Canvas}
import javax.microedition.khronos.opengles.GL10
import scala.util.control.Breaks.{break, breakable}
import org.bigsleep.geometry.{Vec2, Shape, Circle, Rectangle, GeoUtil}

abstract class Widget(val shape : Shape)
{
    def onTouchEvent(s : TouchEvent) : Unit
    def draw(gl : GL10) : Unit
}

class Button(
    s : Shape,
    d : ((GL10, Button) => Unit))
    extends Widget(s)
{
    private var pushed_ = false
    private var onPush = () => {}
    private var onRelease = () => {}
    private var drawer = d
    
    def pushed = pushed_
    
    def setOnPush(f : (() => Unit)) : Unit = onPush = f
    
    def setOnRelease(f : (() => Unit)) : Unit = onRelease = f
    
    def onTouchEvent(e : TouchEvent) : Unit =
    {
        //Log.d("bigsleep", "Button.onTouchEvent")
        val s = e.state
        s.stateType match
        {
            case TouchStateType.DOWN if !pushed =>
            {
                //Log.d("bigsleep", "Button onPush")
                pushed_ = true
                s.widget = Some(this)
                onPush()
            }
            case TouchStateType.UP =>
            {
                //Log.d("bigsleep", "Button onRelease")
                pushed_ = false
                s.widget = None
                onRelease()
            }
            case TouchStateType.MOVE =>
            {
                //Log.d("bigsleep", "Button MOVE")
            }
            case TouchStateType.NON =>
            {
                //Log.d("bigsleep", "Button.onTouchEvent : " + s.stateType)
                pushed_ = false
                s.widget = None
                onRelease()
            }
            case _ => {}
        }
    }
    
    def draw(gl : GL10) : Unit = drawer(gl, this)
}

class Knob(
    s : Shape,
    d : ((GL10, Knob) => Unit),
    min_ : Int = 0,
    max_ : Int = 127,
    ini : Int = 0) extends Widget(s)
{
    private var selected_ = false
    private var pre = math.min(math.max(min_, ini), max_)
    private var cur = pre
    private val drawer = d
    val min = math.min(min_, max_)
    val max = math.max(min_, max_)
    
    def getValue = cur
    
    def selected = selected_
    
    private var onChange = (a : Knob) => {}
    def setOnChange(f : (Knob => Unit)) : Unit = onChange = f
    def setValue(a : Int) : Unit =
    {
        if(a >= min && a <= max){
            cur = a
            onChange(this)
        }
    }
    
    private var sensitivity = 2d
    def setSensitivity(s : Double) : Unit = sensitivity = math.min(math.max(1d, s), 100d)
    
    def onTouchEvent(e : TouchEvent) : Unit =
    {
        val s = e.state
        //Log.d("bigsleep", "Knob.onTouchEvent")
        s.stateType match
        {
            case TouchStateType.DOWN if !selected =>
            {
                //Log.d("bigsleep", "Knob.onTouchEvent DOWN")
                selected_ = true
                s.widget = Some(this)
                pre = cur
            }
            case TouchStateType.UP if selected =>
            {
                //Log.d("bigsleep", "Knob.onTouchEvent UP")
                val dy = s.y - s.downy
                val a = pre + (dy / sensitivity).toInt
                val v = math.min(math.max(min, a), max)
                if(cur != v){
                    cur = v
                    onChange(this)
                }
                selected_ = false
                s.widget = None
            }
            case TouchStateType.MOVE if selected =>
            {
                //Log.d("bigsleep", "Knob.onTouchEvent MOVE")
                val dy = s.y - s.downy
                val a = pre + (dy / sensitivity).toInt
                val v = math.min(math.max(min, a), max)
                if(cur != v){
                    //Log.d("bigsleep", "Change :" + v)
                    cur = v
                    onChange(this)
                }
            }
            case _ => {}
        }
    }
    
    def draw(gl : GL10) : Unit = drawer(gl, this)
}

class AbsoluteLayout(s : Rectangle, usem : Boolean = false) extends Widget(s)
{
    val o = s.leftLower
    val width : Int = s.width
    val height : Int = s.height
    val useMatrix = usem
    private var widgets = Array.empty[(Vec2[Int], Widget)]
    private var drawers = Array.empty[(Vec2[Int], Drawer)]
    private val mat = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
    mat.eraseColor(android.graphics.Color.argb(0, 0, 0, 0))
    
    def push(p : Vec2[Int], w : Widget)
    {
        widgets = widgets :+ (p, w)
        if(useMatrix){
            val n = widgets.size
            //Log.d("bigsleep", "AbsoluteLayout push n " + n)
            w.shape match{
                case Rectangle(q, w1, h1) =>
                {
                    val x1 = p.x + q.x - o.x
                    val y1 = p.y + q.y - o.y
                    val minx = x1
                    val maxx = x1 + w1 - 1
                    val miny = y1
                    val maxy = y1 + h1 - 1
                    val canvas = new Canvas(mat)
                    val paint = new Paint
                    paint.setAlpha(n)
                    paint.setAntiAlias(false)
                    paint.setStyle(Paint.Style.FILL)
                    canvas.drawRect(minx.toFloat, miny.toFloat, maxx.toFloat, maxy.toFloat, paint)
                }
                case Circle(q, r) =>
                {
                    val center = p + q - o
                    val canvas = new Canvas(mat)
                    val paint = new Paint
                    paint.setAlpha(n)
                    paint.setAntiAlias(false)
                    paint.setStyle(Paint.Style.FILL)
                    canvas.drawCircle(center.x, center.y, r, paint)
                }
                case _ => {}
            }
        }
    }
    
    def pushDrawer(p : Vec2[Int], d : Drawer)
    {
        drawers = drawers :+ (p, d)
    }
    
    def onTouchEvent(e : TouchEvent)
    {
        val s = e.state
        //Log.d("bigsleep", "AbsoluteLayout touchevent " + s.x + " " + s.y + " " + s.stateType)
        if(useMatrix){
            s.stateType match{
                case TouchStateType.DOWN =>
                breakable{
                    val x = e.x.toInt - o.x
                    val y = e.y.toInt - o.y
                    if(x >= 0 && x < width && y >= 0 && y < height){
                        val n = android.graphics.Color.alpha(mat.getPixel(x, y)) - 1
                        if(n >= 0){
                            val a = widgets(n)
                            val p = Vec2(e.x.toInt, e.y.toInt) - a._1
                            val w = a._2
                            w.onTouchEvent(TouchEvent(p.x, p.y, s))
                        }
                    }
                }
                case _ => {}
            }
        }else{
            s.stateType match{
                case TouchStateType.DOWN =>
                breakable{
                    widgets.foreach{a =>
                        val p = Vec2(e.x.toInt, e.y.toInt) - a._1
                        val w = a._2
                        val shape = w.shape
                        if(GeoUtil.within(p, shape)){
                            w.onTouchEvent(TouchEvent(p.x, p.y, s))
                            break
                        }
                    }
                }
                case _ => {}
            }
        }
    }
    
    def draw(gl : GL10)
    {
        val unit = 0x10000
        drawers.foreach{a =>
            val p = a._1
            val d = a._2
            val x = p.x * unit
            val y = p.y * unit
            gl.glTranslatex(x, y, 0)
            d(gl)
            gl.glTranslatex(-x, -y, 0)
        }
        widgets.foreach{a =>
            val p = a._1
            val w = a._2
            val x = p.x * unit
            val y = p.y * unit
            gl.glTranslatex(x, y, 0)
            w.draw(gl)
            gl.glTranslatex(-x, -y, 0)
        }
    }
}


