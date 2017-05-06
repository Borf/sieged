using System;

public static class Helper
{
    public static T ParseEnum<T>(string value) where T: struct
    {
        return (T)Enum.Parse(typeof(T), value);
    }
}