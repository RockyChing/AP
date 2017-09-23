#ifndef _LINUX_REFBASE_TEST_H
#define _LINUX_REFBASE_TEST_H
#include <RefBase.h>


class RefBaseTest : virtual public RefBase {
public:
    RefBaseTest();
    RefBaseTest(int date);
    ~RefBaseTest();

    void setData(int data);
    int getData() const;
private:
    void onFirstRef();
private:
    int mData;
};


#endif
