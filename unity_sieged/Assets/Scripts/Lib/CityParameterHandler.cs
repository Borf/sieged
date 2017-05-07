using System.Collections.Generic;
using System.Linq;

public class CityParameterHandler : Dictionary<HouseDesignation, CityParameter>
{
    public IEnumerable<KeyValuePair<HouseDesignation, CityParameter>> GetSliderParameters()
    {
        return this.Where(p => p.Value.Slider != null);
    }

    public IEnumerable<KeyValuePair<HouseDesignation, CityParameter>> GetHiddenParameters()
    {
        return this.Where(p => p.Value.Slider == null);
    }
}