#include <vector>
#include <iostream>
#include <cmath>
using namespace std;

//You need to fill in the minDefeatingSpeed() function

int minDefeatingSpeed(vector<int>& players, int h)
{
    //Write your code after this line. 
    int left = 1;
    int right = players[0];

    for (int p : players) {
        if (p > right) {
            right = p;
        }
    }

    while (left < right) {
        int mid = left + (right - left) / 2;

        int totalHours = 0;
        for (int p : players) {
            totalHours += (p + mid - 1) / mid;
        }

        if (totalHours <= h) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    //Replace this with a line of code that returns the value of the minimum rate defeating speed, k
    return left;
}

int main()
{
    vector<int> players = { 30, 11, 23, 4, 20 };

    int h = 6;
    cout << minDefeatingSpeed(players, h); 
    return 0;
}

//Write the time complexity of this algorithm and explain in detail how you got this time complexity below this line.
// We take the 3 main parts of the algorithm, the first being finding the largest amount of players in a room, which is O(n)
// The second part is the binary search itself, which is O(log(n))
// The third part is getting the total hours, which is O(n), AND nested inside of the binary search loop
// This comes out to O(n) + O(nlog(n))
// O(nlog(n))