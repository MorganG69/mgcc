//
// Created by Morgan Greenhill on 01/04/2022.
//

#ifndef MGG_8_ERROR_H
#define MGG_8_ERROR_H
bool has_error_occurred(void);
void error (char *err_str);
void file_error(char *err_str);
void debug(char *debug_str);
void warn(char *warn_str);
#endif //MGG_8_ERROR_H
