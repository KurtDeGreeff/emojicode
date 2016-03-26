//
//  fetch.c
//  DefaultPackages
//
//  Created by Theo Weidmann on 26/03/16.
//  Copyright © 2016 Theo Weidmann. All rights reserved.
//

#include "EmojicodeAPI.h"
#include "EmojicodeString.h"
#include <curl/curl.h>
#include <string.h>

#define defaultUserAgent "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_4) AppleWebKit/601.5.17 (KHTML, like Gecko) Version/9.1 Safari/601.5.17"

static size_t appendData(void *buffer, size_t size, size_t nmemb, Thread *thread) {
    size_t fullsize = size * nmemb;
    Data *data = stackGetThis(thread)->value;
    
    if (data->length == 0) {
        Object *bytesObject = newArray(size * nmemb);
        data = stackGetThis(thread)->value;
        data->bytesObject = bytesObject;
        data->bytes = bytesObject->value;
    }
    else {
        Object *bytesObject = resizeArray(data->bytesObject, data->length + size * nmemb);
        data = stackGetThis(thread)->value;
        data->bytesObject = bytesObject;
        data->bytes = bytesObject->value;
    }
    
    memcpy(data->bytes + data->length, buffer, fullsize);
    data->length += fullsize;
    
    return fullsize;
}

static Something dataFromURL(Thread *thread) {
    CURL *curl = curl_easy_init();
    
    const char *url = stringToChar(stackGetVariable(0, thread).object->value);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, defaultUserAgent);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendData);
    
    Object *data = newObject(CL_DATA);
    stackPush(data, 0, 0, thread);
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, thread);
    
    CURLcode res = curl_easy_perform(curl);
    
    data = stackGetThis(thread);
    stackPop(thread);
    
    curl_easy_cleanup(curl);
    
    if(res != CURLE_OK) {
        return NOTHINGNESS;
    }
    
    return somethingObject(data);
}

PackageVersion getVersion(){
    return (PackageVersion){0, 1};
}

ClassMethodHandler handlerPointerForClassMethod(EmojicodeChar cl, EmojicodeChar symbol){
    return dataFromURL;
}

MethodHandler handlerPointerForMethod(EmojicodeChar cl, EmojicodeChar symbol){
    return NULL;
}

InitializerHandler handlerPointerForInitializer(EmojicodeChar cl, EmojicodeChar symbol){
    return NULL;
}

Marker markerPointerForClass(EmojicodeChar cl){
    return NULL;
}

uint_fast32_t sizeForClass(Class *cl, EmojicodeChar name) {
    return 0;
}

Deinitializer deinitializerPointerForClass(EmojicodeChar cl){
    return NULL;
}
