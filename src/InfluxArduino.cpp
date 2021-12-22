/*
MIT License

Copyright (c) 2018 Thomas Bruen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <HTTPClient.h>
#include "InfluxArduino.hpp"

InfluxArduino::InfluxArduino()
{
}

InfluxArduino::~InfluxArduino()
{
}

void InfluxArduino::configure(const char database[],const char host[],const uint16_t port)
{   
    //copy these strings to private class pointers for future use

    _database = new char[strlen(database)+1];
    strcpy(_database,database); //strncpy fails for some reason
    _host = new char[strlen(host)+1];
    strcpy(_host,host);
    _port = port;
}

void InfluxArduino::addCertificate(const char cert[])
{
    //copy these strings to private class pointers for future use

    _cert = new char[strlen(cert)+1];
    strcpy(_cert,cert);
    _isSecure = true;
}

void InfluxArduino::authorize(const char username[], const char password[])
{
    //copy these strings to private class pointers for future use

    _username = new char[strlen(username)+1];
    strcpy(_username,username);
    _password = new char[strlen(password)+1];
    strcpy(_password,password);
    _isAuthorised = true;

}

bool InfluxArduino::write(const char *measurement,const char *fieldString)
{
    return write(measurement,"",fieldString);
}

bool InfluxArduino::write(const char *measurement,const char *tagString,const char *fieldString)
{   
    HTTPClient http;
    char uri[32];
    sprintf(uri, "/write?db=%s", _database);

    if(_isSecure)
    {
       http.begin(_host, _port, uri, _cert);
    }
    else
    {
        http.begin(_host, _port, uri);
    }
    http.addHeader("Content-Type", "text/plain"); // not sure what influx is looking for but this works?

    if(_isAuthorised)
    {
        http.setAuthorization(_username,_password);
    }

    char writeBuf[512]; // ¯\_(ツ)_/¯ 
    if(strlen(tagString) > 0)
    {
        sprintf(writeBuf,"%s,%s %s",measurement,tagString,fieldString); //no comma between tags and fields
    }

    else
    {
        //no tags
        sprintf(writeBuf,"%s %s",measurement,fieldString); //no comma between tags and fields
    }
    Serial.println(writeBuf);
    _latestResponse = http.POST(writeBuf);
    http.end();
    return _latestResponse == 204;
}

int InfluxArduino::getResponse()
{
    return _latestResponse;
}

bool InfluxArduino::isSecure()
{
    return _isSecure;
}