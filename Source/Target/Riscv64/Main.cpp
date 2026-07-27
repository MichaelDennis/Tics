void LoopTest()
{
    int count;

    for (int i = 0; i < 1000; i++) {
        count++;
    }
}

int main()
{
    for (int i = 0; i < 1000; i++) {
        LoopTest();        
    }
}