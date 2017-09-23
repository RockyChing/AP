#include <log.h>

#include "RefBaseTest.h"

RefBaseTest::RefBaseTest(): mData(-1) {
    LOG("RefBaseTest create.");
}

RefBaseTest::RefBaseTest(int data) : mData(data) {
    LOG("RefBaseTest create with data: %d", mData);
}

void RefBaseTest::onFirstRef() {
    LOG("RefBaseTest onFirstRef init.");
}

void RefBaseTest::setData(int data) {
    mData = data;
    LOG("setData: %d", mData);
}

int RefBaseTest::getData() const {
    return mData;
}

RefBaseTest::~RefBaseTest() {
    LOG("RefBaseTest destroy.");
}

