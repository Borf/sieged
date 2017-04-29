using System;
using System.Collections.Generic;

public class Grid
{
    private List<Point> offsets = new List<Point> { new Point(0, 1), new Point(0, -1), new Point(1, 0), new Point(-1, 0) };

    public int MinXWithNeighbors;
    public int MaxXWithNeighbors;
    public int MinYWithNeighbors;
    public int MaxYWithNeighbors;

    public Tile[,] Tiles;
    public Grid(int width, int height)
    {
        Tiles = new Tile[width, height];

        MinXWithNeighbors = width/2;
        MaxXWithNeighbors = width/2;
        MinYWithNeighbors = height/2;
        MaxYWithNeighbors = height/2;

        for(int x = 0; x < width; x++)
        {
            for(int y = 0; y < height; y++)
            {
                Tiles[x, y] = new Tile()
                {
                    Builder = Builder.None,
                    NeighboringHouses = 0
                };
            }
        }
    }

    public int Width { get { return Tiles.GetLength(0);  } }

    public int Height { get { return Tiles.GetLength(1); } }


    public void UpdateTile(Point pos, Builder builder)
    {
        var tile = Tiles[pos.X, pos.Y];

        if (tile.Builder != Builder.Generated && builder == Builder.Generated) // None -> Generated || Player -> Generated
        {
            MinXWithNeighbors = Math.Min(MinXWithNeighbors, pos.X-1);
            MaxXWithNeighbors = Math.Max(MaxXWithNeighbors, pos.X+1);
            MinYWithNeighbors = Math.Min(MinYWithNeighbors, pos.Y-1);
            MaxYWithNeighbors = Math.Max(MaxYWithNeighbors, pos.Y+1);

            if (MinXWithNeighbors < 0)
                MinXWithNeighbors = 0;

            if (MaxXWithNeighbors > Width - 1)
                MaxXWithNeighbors = Width - 1;


            if (MinYWithNeighbors < 0)
                MinYWithNeighbors = 0;

            if (MaxYWithNeighbors > Height - 1)
                MaxYWithNeighbors = Height - 1;

            UpdateNeighbors(pos, 1);
        }

        if (tile.Builder == Builder.Generated && builder != Builder.Generated) // Generated -> None || Generated -> Player
            UpdateNeighbors(pos, -1);

        tile.Builder = builder;
    }

    private void UpdateNeighbors(Point pos, int delta)
    {
        foreach(var offset in offsets)
        {
            if (IsOutOfBounds(pos + offset))
                continue;

            Tiles[pos.X + offset.X, pos.Y + offset.Y].NeighboringHouses += delta;
        }
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

    public List<Point> GetEmptyNeighbors()
    {
        var result = new List<Point>();

        for (int x = MinXWithNeighbors; x <= MaxXWithNeighbors; x++)
        {
            for (int y = MinYWithNeighbors; y <= MaxYWithNeighbors; y++)
            {
                if (!Tiles[x, y].HasBuilding && Tiles[x, y].NeighboringHouses > 0)
                    result.Add(new Point(x, y));
            }
        }
        return result;
    }
}