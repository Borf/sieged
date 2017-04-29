public class Grid
{
    public Tile[,] Tiles;
    public Grid(int width, int height)
    {
        Tiles = new Tile[width, height];

        for(int x = 0; x < width; x++)
        {
            for(int y = 0; y < height; y++)
            {
                Tiles[x, y] = new Tile()
                {
                    Builder = Builder.None,
                    Neighbors = 0,
                    Building = null
                };
            }
        }
    }

    public int Width { get { return Tiles.GetLength(0);  } }

    public int Height { get { return Tiles.GetLength(1); } }

    public Tile this[Point p]
    {
        get { return Tiles[p.X, p.Y]; }
        set { Tiles[p.X, p.Y] = value; }
    }

    public Tile this[int x, int y]
    {
        get { return Tiles[x, y]; }
        set { Tiles[x, y] = value; }
    }


    public bool IsOutOfBounds(Point pos)
    {
        return pos.X < 0 || pos.X >= this.Width || pos.Y < 0 || pos.Y >= this.Height;
    }

    public bool IsEmpty(Point pos)
    {
        if (IsOutOfBounds(pos))
            return false;

        return !this.Tiles[pos.X, pos.Y].HasBuilding;
    }
}