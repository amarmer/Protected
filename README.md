# Protected
Class Protected combines data and synchronization objects and allows only synchronized data access.


When a data variable needs to be synchronized, in addition to the data variable, 
added synchronization variable (like mutex, etc.).
Class Protected combines these 2 variables and doesn't allow to use the data variable without synchronization.

For instance:
```C++
// Methods of data cannot accessed since Protected uses private inheritance.
ProtectedByCriticalSection<vector<int>> protectedData = {1, 2, 3};

// In order to access methods, should be called 'Lock()', it returns 'Locker' object, 
// which allows to access protectedData via it's pointer. 
// Constructor of 'Locker' locks synchronization object and it's destructor unlocks it.
auto pData = protectedData.Lock();

pData->push_back(1);

// Or could be used dereference to data
auto& data = *pData;
data.push_back(1);

// Or
protectedData.Lock()->push_back(1);
```
