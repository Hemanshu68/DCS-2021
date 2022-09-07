#pragma once
#pragma once
#include<iostream>
#include<string>
#include<bitset>
using namespace std;

#define key "100000111" //9 char as polynomial is x^8 + x^2 + x^1 + 1
#define zeros "000000000" //will be used while divsion if it is not divisible

string xor8(string a, string b = zeros)
{


    string result = "";

    for (int i = 1; i < a.length(); i++)
    {
        result += a[i] == b[i] ? '0' : '1';

    }
    return result;
}


string mod2div(string divident, string divisor = key)
{

    int i = divisor.length();

    string rem = divident.substr(0, i);

    int n = divident.length();

    //perform division and add one more bit for next dision
    while (i < n)
    {
        if (rem[0] == '1')
            rem = xor8(rem, divisor) + divident[i];
        else
            rem = xor8(rem) + divident[i];

        i += 1;
    }

    // when last bit is added we perform last time the division
    if (rem[0] == '1')
        rem = xor8(rem, divisor);
    else
        rem = xor8(rem);

    return rem;
}


string mod2sub(string a, string b) {
    reverse(a.begin(), a.end());
    reverse(b.begin(), b.end());

    for (int i = 0;i < b.length();i++) {
        a[i] = a[i] == b[i] ? '0' : '1';
    }
    reverse(a.begin(), a.end());
    return a;
}



string crc8send(string msg)
{

    //converting string to bits
    string T = "";
    for (auto ch : msg)///each character is 1 byte
    {
        bitset<8> bits(ch);
        T += bits.to_string();
    }
    for (int i = 1;i <= sizeof(key) - 1;i++) {
        T += '0';
    }
    string rem = mod2div(T);
    string P = mod2sub(T, rem);
    return P;
}

bool crc8recv(string msg) {
    string rem = mod2div(msg);
    if (stoi(rem) != 0)
        return false;
    return true;
}
