using System;
using System.Collections.Generic;
using System.Linq;

public static class Helper
{
    public static T ParseEnum<T>(string value) where T: struct
    {
        return (T)Enum.Parse(typeof(T), value);
    }

    public static IEnumerable<T> GetEnumValues<T>()
    {
        return Enum.GetValues(typeof(T)).Cast<T>();
    }
}