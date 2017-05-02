using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Assets.Scripts.Lib
{
    public static class Math
    {

        private static Random rng = new Random();

        public static ICollection<T> Shuffle<T>(this ICollection<T> list)
        {
            var newList = list.ToList();

            int n = newList.Count;
            while (n > 1)
            {
                n--;
                int k = rng.Next(n + 1);
                T value = newList[k];
                newList[k] = newList[n];
                newList[n] = value;
            }

            return list;
        }

        public static T GetRandomElement<T>(this ICollection<T> collection)
        {
            return collection.ElementAt(UnityEngine.Random.Range(0, collection.Count - 1));
        }
    }
}
