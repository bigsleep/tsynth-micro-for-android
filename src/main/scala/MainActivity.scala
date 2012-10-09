package org.bigsleep.tsynth_micro

import android.app.Activity
import android.os.Bundle
import android.os.Handler
import android.opengl.GLSurfaceView
import android.view.View
import android.view.MotionEvent
import android.view.ViewGroup.LayoutParams
import android.content.Context
import android.content.res.Resources
import android.util.Log

import javax.microedition.khronos.opengles.GL10

import org.bigsleep.android.view.{ResourceImageDrawer, GLRenderer}
import org.bigsleep.android.view.{TouchState, TouchStateType, TouchEvent}
import org.bigsleep.geometry.Vec2

class MainActivity extends Activity with TypedActivity
{
    override def onCreate(bundle: Bundle)
    {
        super.onCreate(bundle)
        requestWindowFeature(android.view.Window.FEATURE_NO_TITLE)
        Main.onActivityCreate(this)
    }

    override def onTouchEvent(ev : MotionEvent) : Boolean =
    {
        Main.onTouchEvent(ev)
        true
    }
    
    override def onStart() : Unit =
    {
        Log.d("bigsleep", "MainActivity.onStart")
        super.onStart()
        Main.onStart()
    }
    
    override def onRestart() : Unit =
    {
        Log.d("bigsleep", "MainActivity.onRestart")
        super.onRestart()
        Main.onRestart()
    }
    
    override def onDestroy() : Unit =
    {
        Log.d("bigsleep", "MainActivity.onDestroy")
        super.onDestroy()
        Main.onDestroy()
    }
    
    override def onBackPressed : Unit =
    {
        Log.d("bigsleep", "MainActivity.onBackPressed")
    }
    
    override def onResume : Unit =
    {
        Log.d("bigsleep", "MainActivity.onResume")
        super.onResume()
        Main.onResume()
    }
    
    override def onPause : Unit =
    {
        Log.d("bigsleep", "MainActivity.onPause")
        super.onPause()
        Main.onPause()
    }
    
    private var windowWidth : Int = -1
    private var windowHeight : Int = -1
    def getWindowWidth : Int =
    {
        if(windowWidth < 0){
            windowWidth = getWindowManager.getDefaultDisplay.getWidth
        }
        windowWidth
    }
    def getWindowHeight : Int =
    {
        if(windowHeight < 0){
            windowHeight = getWindowManager.getDefaultDisplay.getHeight
        }
        windowHeight
    }
}

object Main
{
    Log.d("bigsleep", "Main initialize")
    val width = 1280
    val height = 720
    val margin = 100
    val marginY = 200
    val FPS = 30
    val touchStateSize = 10
    val NOTE_ON = 0x90
    val NOTE_OFF = 0x80
    
    private var glue : Glue = null
    private var activity : MainActivity = null
    private var packageName : String = null
    private var res : Resources = null
    private var view : WidgetView = null
    private var touch : Vector[TouchState] = null
    private var octave = 60
    
    def onActivityCreate(a : MainActivity) : Unit =
    synchronized{
        glue = new Glue
        activity = a
        packageName = a.getPackageName
        res = a.getResources
        ResourceImageDrawer.loadImages(a, res.getStringArray(R.array.drawables))
        view = new WidgetView(a)
        touch = Vector.fill[TouchState](touchStateSize)(new TouchState)
        a.setContentView(view.getAndroidView)
    }
    
    def onStart() : Unit = {view.loadParameters; glue.start()}
    def onRestart() : Unit = {view.loadParameters; glue.start()}
    def onPause() : Unit = {glue.stop(); view.saveParameters()}
    def onResume() : Unit = {view.loadParameters; glue.start()}
    def onDestroy() : Unit = {glue.stop(); view.saveParameters()}
    
    def onTouchEvent(ev : MotionEvent) : Unit =
    {
        synchronized{
            try{
                val pcount = ev.getPointerCount
                val action = ev.getActionMasked
                val id = ev.getActionIndex
                val pid = ev.getPointerId(id)
                val pressure = ev.getPressure(id)
                //Log.d("bigsleep", "touch pointerId: " + pid + " ActionIndex: " + id + " pressure: " + pressure)
                
                val t = action match{
                    case MotionEvent.ACTION_DOWN => TouchStateType.DOWN
                    case MotionEvent.ACTION_UP => TouchStateType.UP
                    case MotionEvent.ACTION_MOVE => TouchStateType.MOVE
                    case MotionEvent.ACTION_POINTER_DOWN => TouchStateType.DOWN
                    case MotionEvent.ACTION_POINTER_UP => TouchStateType.UP
                    case _ => TouchStateType.NON
                }
                val ww = getWindowWidth
                val wh = getWindowHeight
                val fx = width.toDouble / ww.toDouble
                val fy = height.toDouble / wh.toDouble
                
                action match{
                    case MotionEvent.ACTION_MOVE =>
                    {
                        val count = ev.getPointerCount
                        (0 to count - 1).foreach{i =>
                            val pid_ = ev.getPointerId(i)
                            val s = touch(pid_)
                            s.widget match
                            {
                                case Some(w) if s.stateType != TouchStateType.UP &&
                                                s.stateType != TouchStateType.NON =>
                                {
                                    val x = ev.getX(i).toDouble * fx
                                    val y = (wh - ev.getY(i)).toDouble * fy
                                    s.stateType = t
                                    s.px = s.x
                                    s.py = s.y
                                    s.x = x
                                    s.y = y
                                    w.onTouchEvent(TouchEvent(x, y, s))
                                }
                                case None => {}
                            }
                        }
                    }
                    case _ =>
                    {
                        val x = ev.getX(id).toDouble * fx
                        val y = (wh - ev.getY(id)).toDouble * fy
                        if(pid < touchStateSize){
                            val s = touch(pid)
                            s.stateType = t
                            s.px = s.x
                            s.py = s.y
                            s.x = x
                            s.y = y
                            if(t == TouchStateType.DOWN){
                                s.downx = x
                                s.downy = y
                                s.px = x
                                s.py = y
                            }
                            view.onTouchEvent(TouchEvent(x, y, s))
                            s.widget match
                            {
                                case Some(w) => w.onTouchEvent(TouchEvent(x, y, s))
                                case None => view.onTouchEvent(TouchEvent(x, y, s))
                            }
                        }
                    }
                }
            } catch {
                case e =>
                {
                    val pcount = ev.getPointerCount
                    val action = ev.getActionMasked
                    val id = ev.getActionIndex
                    val pid = ev.getPointerId(id)
                    Log.d("bigsleep", "onTouchEvent bad pointer id: " + pid + " pointer count: " + pcount)
                    Log.d("bigsleep", "onTouchEvent exception what: " + e)
                }
            } finally {}
        }
    }
    
    def getOctave : Int = synchronized{ octave }
    def setOctave(o : Int) : Unit = synchronized{ octave = o }
    def upOctave = synchronized{
        (octave to octave + 13).foreach{x =>
            midiReceive(NOTE_OFF, x, 0)
        }
        if(octave + 12 + 13 <= 127) octave = octave + 12
    }
    def downOctave = synchronized{
        (octave to octave + 13).foreach{x =>
            midiReceive(NOTE_OFF, x, 0)
        }
        if(octave - 12 > 0) octave = octave - 12
    }

    def getWidth : Int = width
    def getHeight : Int = height
    
    def getWindowWidth = synchronized{ activity.getWindowWidth }
    def getWindowHeight = synchronized{ activity.getWindowHeight }
    
    def getFace =
    synchronized{
        android.graphics.Typeface.createFromAsset(activity.getAssets, "dejavu.ttf")
    }
    
    def midiReceive(status : Int, data1 : Int, data2 : Int) : Unit =
        glue.midiReceive(status, data1, data2)
    def setTSynthParameterDouble(mod : Int, port : Int, value : Double) : Unit =
        glue.setParameterDouble(mod, port, value)
    def setTSynthParameterInt(mod : Int, port : Int, value : Int) : Unit =
        glue.setParameterInt(mod, port, value)
}


