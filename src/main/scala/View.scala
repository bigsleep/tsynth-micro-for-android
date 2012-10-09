package org.bigsleep.tsynth_micro

import android.util.Log
import android.opengl.GLSurfaceView
import android.content.Context
import javax.microedition.khronos.opengles.GL10

import org.bigsleep.android.view.{
    GLRenderer,
    Drawer,
    TextDrawer,
    ResourceImageDrawer,
    Widget,
    AbsoluteLayout,
    Button,
    Knob,
    ButtonDrawer,
    KnobDrawer,
    TouchState,
    TouchEvent,
    ShapeDrawer,
    CircleDrawer,
    Brush,
    Color,
    SolidColorStroke,
    SolidColorFill}
import org.bigsleep.geometry.{Vec2, Shape, Rectangle, Circle, PolyLine, GeoUtil}

class WidgetView(ctx : Context)
{
    private val view = new GLSurfaceView(ctx)
    private val renderer = new GLRenderer(Main.width.toDouble, Main.height.toDouble)
    renderer.setOnDraw(this.onDraw _)
    view.setRenderer(renderer)
    val preference = ctx.getSharedPreferences("parameters", Context.MODE_PRIVATE)
    val layout = new AbsoluteLayout(Rectangle(Vec2(0, 0), Main.width, Main.height), true)
    val layout2 = new AbsoluteLayout(Rectangle(Vec2(0, 0), Main.width, Main.height))
    val vco1Control = VCOControl(101, "VCO1")
    val vco2Control = VCOControl(102, "VCO2")
    val vcfControl = EGControl(104, "VCF")
    val vcaControl = EGControl(105, "VCA")
    val keyboard = Keyboard()
    val volume = new Knob(EGControl.getKnobShape, EGControl.getKnobDrawer(_, _), 0, 255, 127)
    val downOctave = OctaveControl.down
    val upOctave = OctaveControl.up
    volume.setOnChange{(k : Knob) => Main.setTSynthParameterDouble(105, 5, (k.getValue.toDouble / k.max.toDouble) * 0.2)}
    volume.setValue(127)
    
    val face = Main.getFace
    val paint = new android.graphics.Paint
    paint.setTextSize(40)
    paint.setColor(0xFF000000)
    paint.setAntiAlias(true)
    paint.setTypeface(face)
    val volText = new TextDrawer("Vol", paint)
    volText.atCenter(true)
    
    //layout.push(Vec2(5, 0), keyboard)
    layout.push(Vec2(10, 560), vco1Control)
    layout.push(Vec2(10, 410), vco2Control)
    layout.push(Vec2(620, 560), vcfControl)
    layout.push(Vec2(620, 410), vcaControl)
    layout.push(Vec2(1140, 566), volume)
    layout.push(Vec2(1110, 410), downOctave)
    layout.push(Vec2(1190, 410), upOctave)
    layout.push(Vec2(0, 0), keyboard)
    layout2.pushDrawer(Vec2(1192, 616), volText);
    loadParameters()
    
    def onDraw(gl : GL10) : Unit =
    {
        gl.glClearColor(1f, 1f, 1f, 1f)
        gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT)
        gl.glMatrixMode(GL10.GL_MODELVIEW)
        
        gl.glLoadIdentity()
        Main.synchronized{
            layout.draw(gl)
            layout2.draw(gl)
        }
    }
    
    def getAndroidView : android.view.View = view
    
    def onTouchEvent(e : TouchEvent) : Unit = layout.onTouchEvent(e)
    
    def loadParameters() : Unit =
    {
        val vol = preference.getInt("volume", volume.max / 2)
        volume.setValue(vol)
        val oc = preference.getInt("octave", Main.getOctave)
        Main.setOctave(oc)
        
        val vco1a = preference.getInt("vco1attack", 10)
        val vco1d = preference.getInt("vco1decay", 0)
        val vco1s = preference.getInt("vco1sustain", 127)
        val vco1r = preference.getInt("vco1release", 10)
        val vco1w = preference.getInt("vco1wavetype", 1)
        vco1Control.getAttack.setValue(vco1a)
        vco1Control.getDecay.setValue(vco1d)
        vco1Control.getSustain.setValue(vco1s)
        vco1Control.getRelease.setValue(vco1r)
        vco1Control.getWaveType.setValue(vco1w)
        
        val vco2a = preference.getInt("vco2attack", 10)
        val vco2d = preference.getInt("vco2decay", 0)
        val vco2s = preference.getInt("vco2sustain", 127)
        val vco2r = preference.getInt("vco2release", 10)
        val vco2w = preference.getInt("vco2wavetype", 1)
        vco2Control.getAttack.setValue(vco2a)
        vco2Control.getDecay.setValue(vco2d)
        vco2Control.getSustain.setValue(vco2s)
        vco2Control.getRelease.setValue(vco2r)
        vco2Control.getWaveType.setValue(vco2w)
        
        val vcfa = preference.getInt("vcfattack", 10)
        val vcfd = preference.getInt("vcfdecay", 0)
        val vcfs = preference.getInt("vcfsustain", 127)
        val vcfr = preference.getInt("vcfrelease", 10)
        vcfControl.getAttack.setValue(vcfa)
        vcfControl.getDecay.setValue(vcfd)
        vcfControl.getSustain.setValue(vcfs)
        vcfControl.getRelease.setValue(vcfr)
        
        val vcaa = preference.getInt("vcaattack", 10)
        val vcad = preference.getInt("vcadecay", 0)
        val vcas = preference.getInt("vcasustain", 127)
        val vcar = preference.getInt("vcarelease", 10)
        vcaControl.getAttack.setValue(vcaa)
        vcaControl.getDecay.setValue(vcad)
        vcaControl.getSustain.setValue(vcas)
        vcaControl.getRelease.setValue(vcar)
    }
    
    def saveParameters() : Unit =
    {
        val e = preference.edit
        e.putInt("volume", volume.getValue)
        e.putInt("octave", Main.getOctave)
        
        e.putInt("vco1attack", vco1Control.getAttack.getValue)
        e.putInt("vco1decay", vco1Control.getDecay.getValue)
        e.putInt("vco1sustain", vco1Control.getSustain.getValue)
        e.putInt("vco1release", vco1Control.getRelease.getValue)
        e.putInt("vco1wavetype", vco1Control.getWaveType.getValue)
        
        e.putInt("vco2attack", vco2Control.getAttack.getValue)
        e.putInt("vco2decay", vco2Control.getDecay.getValue)
        e.putInt("vco2sustain", vco2Control.getSustain.getValue)
        e.putInt("vco2release", vco2Control.getRelease.getValue)
        e.putInt("vco2wavetype", vco2Control.getWaveType.getValue)
        
        e.putInt("vcfattack", vcfControl.getAttack.getValue)
        e.putInt("vcfdecay", vcfControl.getDecay.getValue)
        e.putInt("vcfsustain", vcfControl.getSustain.getValue)
        e.putInt("vcfrelease", vcfControl.getRelease.getValue)
        
        e.putInt("vcaattack", vcaControl.getAttack.getValue)
        e.putInt("vcadecay", vcaControl.getDecay.getValue)
        e.putInt("vcasustain", vcaControl.getSustain.getValue)
        e.putInt("vcarelease", vcaControl.getRelease.getValue)
        e.commit()
    }
}

object SimpleKnobDrawer
{
    def apply(
        center : Vec2[Int],
        radius : Int,
        central : Double,
        brush1 : Brush,
        brush2 : Brush,
        brush3 : Brush) =
    {
        val circle = Circle(Vec2(0, 0), radius)
        val point = Circle(Vec2(0, (radius.toDouble * 0.8).toInt), (radius.toDouble * 0.1).toInt)
        val d1 = CircleDrawer(circle, brush1)
        val d2 = CircleDrawer(point, brush2)
        val d3 = CircleDrawer(circle, brush3)
        val knobDrawer = (gl : GL10, k : Knob) =>
        {
            if(!k.selected) d1(gl)
            else d3(gl)
            d2(gl)
        }
        
        val backDrawer = new Drawer{ def apply(gl : GL10){} }
        
        KnobDrawer(center, radius, central, knobDrawer, backDrawer)
    }
}

abstract class EGControl(s : Shape) extends Widget(s)
{
    def getAttack : Knob
    def getDecay : Knob
    def getSustain : Knob
    def getRelease : Knob
}

object EGControl
{
    val radius = 54
    val width = 480
    val height = 150
    val lineWidth = 2d
    val central = 300d
    val red = Color(0.6d, 0d, 0d, 1d)
    val gray = Color(0.5d, 0.5d, 0.5d, 1d)
    val darkGray = Color(0.2d, 0.2d, 0.2d, 1d)
    val rect : Rectangle = Rectangle(Vec2(0, 0), width, height)
    val circle = Circle(Vec2(radius, radius), radius)
    val brush1 = Brush(SolidColorStroke(lineWidth, darkGray),
                       SolidColorFill(gray))
    val brush2 = Brush(None, Some(SolidColorFill(red)))
    val brush3 = Brush(SolidColorStroke(lineWidth, darkGray),
                       SolidColorFill(Color(0.1176d, 0.5647d, 1.0d, 1d)))
    val brushOut = Brush(SolidColorStroke(lineWidth, darkGray), SolidColorFill(Color.WHITE))
    val knobDrawer = SimpleKnobDrawer(Vec2(radius, radius), radius, central, brush1, brush2, brush3)
    val outlineDrawer = ShapeDrawer(rect, brushOut)
    
    val face = Main.getFace
    val paint = new android.graphics.Paint
    paint.setTextSize(60)
    paint.setColor(0xFF000000)
    paint.setAntiAlias(true)
    paint.setTypeface(face)
    val paint2 = new android.graphics.Paint(paint)
    paint2.setTextSize(36)
    val attackText = new TextDrawer("A", paint)
    val decayText = new TextDrawer("D", paint)
    val sustainText = new TextDrawer("S", paint)
    val releaseText = new TextDrawer("R", paint)
    attackText.atCenter(true)
    decayText.atCenter(true)
    sustainText.atCenter(true)
    releaseText.atCenter(true)
    
    def getKnobShape = circle
    def getKnobDrawer = knobDrawer
    
    def apply(id : Int, title : String) : EGControl =
    {
        val titleText = new TextDrawer(title, paint2)
        titleText.atCenter(true)
        new EGControl(rect)
        {
            private val attack = new Knob(circle, knobDrawer(_,_))
            private val decay = new Knob(circle, knobDrawer(_,_))
            private val sustain = new Knob(circle, knobDrawer(_,_))
            private val release = new Knob(circle, knobDrawer(_,_))
            private val layout = new AbsoluteLayout(rect, true)
            private val layout2 = new AbsoluteLayout(rect)
            private val x0 = 6
            private val y0 = 6
            private val x1 = 60
            private val y1 = 56
            private val b = 120
            
            layout.push(Vec2(x0 + b * 0, y0), attack)
            layout.push(Vec2(x0 + b * 1, y0), decay)
            layout.push(Vec2(x0 + b * 2, y0), sustain)
            layout.push(Vec2(x0 + b * 3, y0), release)
            layout2.pushDrawer(Vec2(x1 + b * 0, y1), attackText)
            layout2.pushDrawer(Vec2(x1 + b * 1, y1), decayText)
            layout2.pushDrawer(Vec2(x1 + b * 2, y1), sustainText)
            layout2.pushDrawer(Vec2(x1 + b * 3, y1), releaseText)
            layout2.pushDrawer(Vec2(width / 2, (height.toDouble * 0.86).toInt), titleText)
            
            attack.setOnChange{(k : Knob) => Main.setTSynthParameterDouble(id, 1, (k.getValue.toDouble / k.max.toDouble) * 1d)}
            decay.setOnChange{(k : Knob) => Main.setTSynthParameterDouble(id, 2, (k.getValue.toDouble / k.max.toDouble) * 1d)}
            sustain.setOnChange{(k : Knob) => Main.setTSynthParameterDouble(id, 3, (k.getValue.toDouble / k.max.toDouble))}
            release.setOnChange{(k : Knob) => Main.setTSynthParameterDouble(id, 4, (k.getValue.toDouble / k.max.toDouble) * 4d)}
            
            def onTouchEvent(e : TouchEvent)
            {
                layout.onTouchEvent(e)
            }
            
            def draw(gl : GL10)
            {
                outlineDrawer(gl)
                layout.draw(gl)
                layout2.draw(gl)
            }
            
            def getAttack = attack
            def getDecay = decay
            def getSustain = sustain
            def getRelease = release
        }
    }
}

object WaveTypeKnob
{
    val face = Main.getFace
    val paint = new android.graphics.Paint
    paint.setTextSize(40)
    paint.setColor(0xFF000000)
    paint.setAntiAlias(true)
    paint.setTypeface(face)
    val sin = new TextDrawer("SIN", paint)
    val saw = new TextDrawer("SAW", paint)
    val tri = new TextDrawer("TRI", paint)
    val squ = new TextDrawer("SQU", paint)
    sin.atCenter(true)
    saw.atCenter(true)
    tri.atCenter(true)
    squ.atCenter(true)
    
    val eg = EGControl
    val drawer1 = SimpleKnobDrawer(Vec2(eg.radius, eg.radius), eg.radius, 270d, eg.brush1, eg.brush2, eg.brush3)
    
    def apply() : Knob = new Knob(Circle(Vec2(eg.radius, eg.radius), eg.radius), this.draw(_, _), 1, 4, 1)
    
    def draw(gl : GL10, k : Knob) : Unit =
    {
        drawer1(gl, k)
        val x = 0x10000 * 54
        val y = 0x10000 * 54
        gl.glTranslatex(x, y, 0)
        k.getValue match{
            case 1 => sin(gl)
            case 2 => saw(gl)
            case 3 => tri(gl)
            case 4 => squ(gl)
            case _ => {}
        }
        gl.glTranslatex(-x, -y, 0)
    }
}


abstract class VCOControl(s : Shape) extends Widget(s)
{
    def getAttack : Knob
    def getDecay : Knob
    def getSustain : Knob
    def getRelease : Knob
    def getWaveType : Knob
}

object VCOControl
{
    val radius = 54
    val width = 600
    val height = 150
    val lineWidth = 2d
    val rect : Rectangle = Rectangle(Vec2(0, 0), width, height)
    val circle = Circle(Vec2(radius, radius), radius)
    val darkGray = Color(0.2d, 0.2d, 0.2d, 1d)
    val brushOut = Brush(SolidColorStroke(lineWidth, darkGray), SolidColorFill(Color.WHITE))
    val outlineDrawer = ShapeDrawer(rect, brushOut)
    val knobDrawer = EGControl.getKnobDrawer
    
    val face = Main.getFace
    val paint = new android.graphics.Paint
    paint.setTextSize(60)
    paint.setColor(0xFF000000)
    paint.setAntiAlias(true)
    paint.setTypeface(face)
    val paint2 = new android.graphics.Paint(paint)
    paint2.setTextSize(36)
    val attackText = new TextDrawer("A", paint)
    val decayText = new TextDrawer("D", paint)
    val sustainText = new TextDrawer("S", paint)
    val releaseText = new TextDrawer("R", paint)
    attackText.atCenter(true)
    decayText.atCenter(true)
    sustainText.atCenter(true)
    releaseText.atCenter(true)
    
    def apply(id : Int, title : String) : VCOControl =
    {
        val titleText = new TextDrawer(title, paint2)
        titleText.atCenter(true)
        new VCOControl(rect)
        {
            private val attack = new Knob(circle, knobDrawer(_,_))
            private val decay = new Knob(circle, knobDrawer(_,_))
            private val sustain = new Knob(circle, knobDrawer(_,_))
            private val release = new Knob(circle, knobDrawer(_,_))
            private val waveType = WaveTypeKnob()
            private val layout = new AbsoluteLayout(rect, true)
            private val layout2 = new AbsoluteLayout(rect)
            private val x0 = 6
            private val y0 = 6
            private val x1 = 60
            private val y1 = 56
            private val b = 120
            waveType.setSensitivity(60d)
            
            layout.push(Vec2(x0 + b * 0, y0), waveType)
            layout.push(Vec2(x0 + b * 1, y0), attack)
            layout.push(Vec2(x0 + b * 2, y0), decay)
            layout.push(Vec2(x0 + b * 3, y0), sustain)
            layout.push(Vec2(x0 + b * 4, y0), release)
            layout2.pushDrawer(Vec2(x1 + b * 1, y1), attackText)
            layout2.pushDrawer(Vec2(x1 + b * 2, y1), decayText)
            layout2.pushDrawer(Vec2(x1 + b * 3, y1), sustainText)
            layout2.pushDrawer(Vec2(x1 + b * 4, y1), releaseText)
            layout2.pushDrawer(Vec2(width / 2, (height.toDouble * 0.86).toInt), titleText)
            
            attack.setOnChange{(k : Knob) => Main.setTSynthParameterDouble(id, 1, (k.getValue.toDouble / k.max.toDouble) * 1d)}
            decay.setOnChange{(k : Knob) => Main.setTSynthParameterDouble(id, 2, (k.getValue.toDouble / k.max.toDouble) * 1d)}
            sustain.setOnChange{(k : Knob) => Main.setTSynthParameterDouble(id, 3, (k.getValue.toDouble / k.max.toDouble))}
            release.setOnChange{(k : Knob) => Main.setTSynthParameterDouble(id, 4, (k.getValue.toDouble / k.max.toDouble) * 4d)}
            waveType.setOnChange{(k : Knob) => Main.setTSynthParameterInt(id, 1, k.getValue) }
            
            def onTouchEvent(e : TouchEvent)
            {
                layout.onTouchEvent(e)
            }
            
            def draw(gl : GL10)
            {
                outlineDrawer(gl)
                layout.draw(gl)
                layout2.draw(gl)
            }
            
            def getAttack = attack
            def getDecay = decay
            def getSustain = sustain
            def getRelease = release
            def getWaveType = waveType
        }
    }
}

abstract class Keyboard(s : Shape) extends Widget(s)

object Keyboard
{
    val width = 1280
    val height = 400
    val rect = Rectangle(Vec2(0, 0), width, height)
    val key = Rectangle(Vec2(5, 5), 150, 190)
    val key2 = Rectangle(Vec2(5, 5), 75, 190)
    val lineWidth = 4d
    val red = Color(0.6d, 0d, 0d, 1d)
    val gray = Color(0.5d, 0.5d, 0.5d, 1d)
    val darkGray = Color(0.2d, 0.2d, 0.2d, 1d)
    val brush = Brush(SolidColorStroke(lineWidth, darkGray), SolidColorFill(Color.WHITE))
    val brush2 = Brush(SolidColorStroke(lineWidth, darkGray), SolidColorFill(Color(0.1176d, 0.5647d, 1.0d, 1d)))
    val drawer = ResourceImageDrawer(R.drawable.key)
    val drawerp = ResourceImageDrawer(R.drawable.key_pushed)
    drawer.setScale(0.5d)
    drawerp.setScale(0.5d)
    
    def apply() : Keyboard = new Keyboard(rect){
        val keys = (0 to 13).map{x =>
                new Button(key, ButtonDrawer(drawer, drawerp)(_, _))
            }.toArray
        val layout = new AbsoluteLayout(rect, true)
        val position = Array(
            Vec2(0, 0),
            Vec2(80, 200),
            Vec2(160, 0),
            Vec2(240, 200),
            Vec2(320, 0),
            Vec2(480, 0),
            Vec2(560, 200),
            Vec2(640, 0),
            Vec2(720, 200),
            Vec2(800, 0),
            Vec2(880, 200),
            Vec2(960, 0),
            Vec2(1120, 0),
            Vec2(1200, 200))
        (0 to 13).foreach{x =>
            layout.push(position(x), keys(x))
        }
        (0 to 13).foreach{x =>
            keys(x).setOnPush{() => Main.midiReceive(Main.NOTE_ON, Main.getOctave + x, 127)}
            keys(x).setOnRelease{() => Main.midiReceive(Main.NOTE_OFF, Main.getOctave + x, 0)}
        }
        
        def setOnPush(k : Int, f : (() => Unit)) : Unit = if(k < 14) keys(k).setOnPush(f)
        def setOnRelease(k : Int, f : (() => Unit)) : Unit = if(k < 14) keys(k).setOnRelease(f)
        
        def onTouchEvent(e : TouchEvent)
        {
            layout.onTouchEvent(e)
        }
        
        def draw(gl : GL10)
        {
            layout.draw(gl)
        }
    }
}

object OctaveControl
{
    val rect = Rectangle(Vec2(0, 0), 80, 130)
    val left = PolyLine(List(Vec2(70, 110), Vec2(10, 60), Vec2(70, 20)))
    val right = PolyLine(List(Vec2(10, 110), Vec2(70, 60), Vec2(10, 20)))
    val lineWidth = 2d
    val gray = Color(0.5d, 0.5d, 0.5d, 1d)
    val darkGray = Color(0.2d, 0.2d, 0.2d, 1d)
    val brush = Brush(SolidColorStroke(lineWidth, darkGray), SolidColorFill(Color.WHITE))
    val brush2 = Brush(SolidColorStroke(lineWidth, darkGray), SolidColorFill(Color(0.1176d, 0.5647d, 1.0d, 1d)))
    val unpush = ShapeDrawer(rect, brush)
    val push = ShapeDrawer(rect, brush2)
    val drawerLeft = ShapeDrawer(left, brush)
    val drawerRight = ShapeDrawer(right, brush)
    
    val drawUp = new Drawer
    {
        def apply(gl : GL10)
        {
            unpush(gl)
            drawerRight(gl)
        }
    }
    
    val drawUpp = new Drawer
    {
        def apply(gl : GL10)
        {
            push(gl)
            drawerRight(gl)
        }
    }
    
    val drawDown = new Drawer
    {
        def apply(gl : GL10)
        {
            unpush(gl)
            drawerLeft(gl)
        }
    }
    
    val drawDownp = new Drawer
    {
        def apply(gl : GL10)
        {
            push(gl)
            drawerLeft(gl)
        }
    }
    
    def up : Button =
    {
        val b = new Button(rect, ButtonDrawer(drawUp, drawUpp)(_, _))
        b.setOnRelease{() => Main.upOctave}
        b
    }
    
    def down : Button =
    {
        val b = new Button(rect, ButtonDrawer(drawDown, drawDownp)(_, _))
        b.setOnRelease{() => Main.downOctave}
        b
    }
}





