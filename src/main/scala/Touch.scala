package org.bigsleep.android.view

case class TouchEvent(
    val x : Double, // relative coordinate
    val y : Double, // relative coordinate
    val state : TouchState)

class TouchStateType
{
    override def toString =
    {
        this match{
            case TouchStateType.DOWN => "DOWN"
            case TouchStateType.UP => "UP"
            case TouchStateType.MOVE => "MOVE"
            case TouchStateType.NON => "NON"
            case _ => "OTHER"
        }
    }
}

object TouchStateType
{
    val DOWN, MOVE, UP, OUTSIDE, NON = new TouchStateType
}

class TouchState
{
    var stateType = TouchStateType.NON
    var x = 0d
    var y = 0d
    var px = 0d
    var py = 0d
    var downx = 0d
    var downy = 0d
    var tap = false
    var widget : Option[Widget] = None
}

