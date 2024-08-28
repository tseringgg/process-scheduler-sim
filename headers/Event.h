class Event {
public:
    int type;
    int eventTime;
    int pid;

    // Constructor
    Event(int ty, int event, int processId) : type(ty), eventTime(event), pid(processId) {}
    Event() {}
};