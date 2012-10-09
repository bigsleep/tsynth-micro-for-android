package org.bigsleep.android.view

import android.util.Log
import javax.microedition.khronos.opengles.GL10
import org.bigsleep.geometry.{Vec2}

case class ButtonDrawer(unpushed : Drawer, pushed : Drawer)
{
    def apply(gl : GL10, a : Button) : Unit =
    {
        if(a.pushed) pushed(gl)
        else unpushed(gl)
    }
}

case class KnobDrawer(
    center : Vec2[Int],
    radius : Int,
    centralAngle : Double,
    knob : ((GL10, Knob) => Unit),
    back : Drawer)
{
    def apply(gl : GL10, a : Knob)
    {
        back(gl)
        val unit = 0x10000
        val x = center.x * unit
        val y = center.y * unit
        val v = (a.getValue - a.min).toDouble / (a.max - a.min).toDouble * centralAngle
        val ang = ((centralAngle * 0.5d - v) * unit.toDouble).toInt
        gl.glTranslatex(x, y, 0)
        gl.glRotatex(ang, 0, 0, 0x10000)
        knob(gl, a)
        gl.glRotatex(-ang, 0, 0, 0x10000)
        gl.glTranslatex(-x, -y, 0)
    }
}

