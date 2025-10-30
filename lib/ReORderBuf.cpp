#include "ReOrderBuf.hpp"

ReOrderBuf::ReOrderBuf(int n)
{
    size = n;
    n = 0;
    head = 0;
    tail = 0;
    table = new ReOrderBuf_entry[n];
}