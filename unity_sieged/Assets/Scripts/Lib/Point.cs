public class Point
{
    public int X;
    public int Y;

    public Point(int x, int y)
    {
        X = x;
        Y = y;
    }

    public static Point operator +(Point p1, Point p2)
    {
        return new Point(p1.X + p2.X, p1.Y + p2.Y);
    }

    public override int GetHashCode()
    {
        return 10000 * X + Y;
    }

    public override bool Equals(object obj)
    {
        if (obj == null)
            return false;

        var point = (Point)obj;

        if (point == null)
            return false;

        return this.X == point.X && this.Y == point.Y;
    }
}
