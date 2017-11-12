import datetime
from astral import Astral
from dateutil import tz

city_name = 'San Francisco'

city = Astral()[city_name]

print ('// Sunrise and Sunset times for the location:')
print ('//   City = %s' % city_name)
print ('//   Region = %s' % city.region)
print ('//   Timezone = %s' % city.timezone)
print ('//   Lat = %.6f / Long = %.6f' % \
    (city.latitude, city.longitude))
print ('// Computed with the Python Astral module v1.4')
print ('// Table defined as follow:')
print ('//   - Each line correspond to 1 day of the year (365 lines)')
print ('//   - Colums are:')
print ('//     0: sunrise hour')
print ('//     1: sunrise minute')
print ('//     2: sunset hour')
print ('//     3: sunset minute')
print ('// Times are Local, but ignore daylight saving!')
print ('// (local is good for humans, no DST it good for our simple RTC)')
print ('// Times are computed for the year 2017, so there maybe 1-2 minutes')
print ('// error for other years, and 2-3 minutes for leap years.')
print ('const uint8_t sunset_sunrise_times [][] = {')

utc_tz = tz.gettz('UTC')
local_tz = tz.gettz(city.timezone)

for day in range(1, 366):
    date = (datetime.date.fromordinal(day)).replace(year=2017)
    sun = city.sun(date, local=False)
    if day != 365:
        sep = ','
    else:
        sep = ' '
    # Get sunrise and sunset times in UTC (see "local=False" above)
    sunrise = sun['sunrise']
    sunset = sun['sunset']
    # Convert to local (Atral drop the tzinfo when using local, so DST is not
    # accessible --> we need to redo the job ourselves)
    utc_sunrise = sunrise.replace(tzinfo=utc_tz)
    utc_sunset = sunset.replace(tzinfo=utc_tz)
    local_sunrise = utc_sunrise.astimezone(local_tz)
    local_sunset = utc_sunset.astimezone(local_tz)
    local_sunrise_nodst = local_sunrise - local_sunrise.dst()
    local_sunset_nodst = local_sunset - local_sunset.dst()
    comment = '// day=%03d (%d-%02d-%02d)' % (day, date.year, date.month, date.day)
    if local_sunrise.dst() > datetime.timedelta(seconds=1) :
            comment = comment + ' [DST ignored!]'
    print ('  {%d, %d, %d, %d}%c\t%s' % (
        local_sunrise_nodst.hour, local_sunrise_nodst.minute,
        local_sunset_nodst.hour, local_sunset_nodst.minute, sep, comment))
print ('};')

# for month in range(1, 13):
#     date_in = datetime.date(2017, month, 1)
#     day = date_in.timetuple().tm_yday
#     print('1st of month %d -> yday=%d' % (month, day))
#     date_out = (datetime.date.fromordinal(day)).replace(year=2017)
#     print(date_out)
