#if ! defined( IIRFILTER_CLASS_HEADER )
#define IIRFILTER_CLASS_HEADER

enum
{
    IIRFILTER_TYPE_LINEAR = 1 << 0,
    IIRFILTER_TYPE_DEG = 1 << 1,
    IIRFILTER_TYPE_RAD = 1 << 2
};

class iirfilter
{
public:

//    iirfilter() {setFC(0.5); type = IIRFILTER_TYPE_LINEAR; reset();};

    iirfilter(double fc = 0.5, int tp = IIRFILTER_TYPE_LINEAR);
    ~iirfilter(){};
    double filter(double data);
    void reset(double a = 0.0);
    void setFC(double fc = 0.1);
    void setType(int tp);
    double getFc(void);
    int getType(void);
    double get(void);

protected:

    void unwrapDeg(double deg);
    void unwrapRad(double rad);

private:

    double a0;
    double b1;
    double accum;
    double oldDeg;
    double oldRad;
    int wraps;
    int type;
};

#endif