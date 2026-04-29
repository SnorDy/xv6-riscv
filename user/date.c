#include "kernel/types.h"
#include "user/user.h"

static const int month_days[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static const char *month_names[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int days_in_month(int month, int year)
{
    if (month == 1 && is_leap_year(year))
        return 29;
    return month_days[month];
}

static void print_padded2(int val)
{
    if (val < 10)
        printf("0");
    printf("%d", val);
}

static void print_padded3(int val)
{
    if (val < 100)
        printf("0");
    if (val < 10)
        printf("0");
    printf("%d", val);
}

static void print_padded9(int val)
{
    if (val < 100000000) printf("0");
    if (val < 10000000)  printf("0");
    if (val < 1000000)   printf("0");
    if (val < 100000)    printf("0");
    if (val < 10000)     printf("0");
    if (val < 1000)      printf("0");
    if (val < 100)       printf("0");
    if (val < 10)        printf("0");
    printf("%d", val);
}

static void seconds_to_datetime(uint64 seconds, int *year, int *month, int *day,
                    int *hour, int *minute, int *second)
{
    *second = seconds % 60;
    seconds /= 60;
    *minute = seconds % 60;
    seconds /= 60;
    *hour = seconds % 24;
    seconds /= 24;
    
    uint64 days = seconds;
    int y = 1970;
    
    while (1) {
        int days_in_year = is_leap_year(y) ? 366 : 365;
        if (days < (uint64)days_in_year)
            break;
        days -= days_in_year;
        y++;
    }
    *year = y;
    
    int m = 0;
    while (1) {
        int dim = days_in_month(m, y);
        if (days < (uint64)dim)
            break;
        days -= dim;
        m++;
    }
    *month = m;
    *day = (int)days + 1;
}

int main(void)
{
    uint64 nanoseconds = rtc();
    
    uint64 seconds = nanoseconds / 1000000000ULL;
    uint64 fractional = nanoseconds % 1000000000ULL;
    
    int millis = (int)(fractional / 1000000);
    int micros = (int)((fractional / 1000) % 1000);
    int nanos = (int)(fractional % 1000);
    
    int year, month, day, hour, minute, second;
    seconds_to_datetime(seconds, &year, &month, &day, &hour, &minute, &second);
    
    printf("%s %d, %d  ", month_names[month], day, year);
    print_padded2(hour);
    printf(":");
    print_padded2(minute);
    printf(":");
    print_padded2(second);
    printf(".");
    print_padded3(millis);
    printf("\n");
    
    printf("%d-", year);
    print_padded2(month + 1);
    printf("-");
    print_padded2(day);
    printf("T");
    print_padded2(hour);
    printf(":");
    print_padded2(minute);
    printf(":");
    print_padded2(second);
    printf(".");
    print_padded9((int)fractional);
    printf("\n");
    printf("ms: %d, us: %d, ns: %d\n", millis, micros, nanos);
    
    exit(0);
}
