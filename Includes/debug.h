//
// Created by Ugo WAREMBOURG on 15/12/2024.
//

#ifndef MESSAGERIEPROJECTAP3_DEBUG_H
#define MESSAGERIEPROJECTAP3_DEBUG_H

#include <stdio.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "[DEBUG] %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

#endif //MESSAGERIEPROJECTAP3_DEBUG_H
