//
// Created by Sebastian Balz on 12.03.2018.
//

#include <string>
#include <logging.h>
#include <unistd.h>

using  namespace std;

/**
 * wast some time
 */
void heavyCalculation(){
    Log::log(__FUNCTION__,"start heavy calc",Info);

    int time = rand()%10000;
    Log::log(__FUNCTION__,"calculation does need " + to_string(time) + "us",Debug);
    usleep(time);
    Log::log(__FUNCTION__,"calculation done",Message);
}
/**
 * random example function
 * @param abc string
 * @param num number added to string
 * @return string + num
 */
string magicFunction(string abc, int num){
    if(num < 0){
        Log::log(__FUNCTION__,"not allowed parameter : " + to_string(num),CriticError);
        return "";
    }
    Log::log("magicFunction","got param : " + abc + " and " + to_string(num),Debug);
    heavyCalculation();
    string out = abc + to_string(num);
    Log::log("magicFunction","magically result : " + out,Message);
    return out;

}
int main(){
    Log::log("main","do something with default logLevel",UserInfo);
    magicFunction("abc",42);
    magicFunction("hello world",-1);

    Log::setLogLevel(Message,None);
    Log::log("main","do something with Message as logLevel",UserInfo);
    magicFunction("abc",42);
    magicFunction("hello world",-1);

    Log::setLogLevel(DebugL3,None);
    Log::log("main","do something with Debug as logLevel",UserInfo);
    magicFunction("abc",42);
    magicFunction("hello world",-1);

}