package org.bigsleep.geometry

abstract class Shape

case class Circle(val center : Vec2[Int], val radius : Int) extends Shape()

case class Rectangle(val leftLower : Vec2[Int], val width : Int, val height : Int) extends Shape()

case class PolyLine(val points : List[Vec2[Int]]) extends Shape()

object GeoUtil
{
    def within(p : Vec2[Int], s : Shape) : Boolean =
    {
        s match{
            case x @ Circle(_, _)       => within_(p, x)
            case x @ Rectangle(_, _, _) => within_(p, x)
        }
    }
    
    def within_(p : Vec2[Int], s : Circle) : Boolean =
    {
        val r = s.radius
        val c = s.center
        (p - c).normSq <= r * r
    }
    
    def within_(p : Vec2[Int], s : Rectangle) : Boolean =
    {
        val left = s.leftLower.x
        val right = left + s.width
        val bottom = s.leftLower.y
        val top = bottom + s.height
        left <= p.x && p.x <= right && bottom <= p.y && p.y <= top
    }
}
