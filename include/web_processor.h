/* processor is the centralized function for handling templates
*  because the logic and template are interwoven, its difficult
*  separate them out as in serve/cards. All templates must have logic
*  in processor to define the template value.
*/
#ifndef WEB_PROCESSOR
#define WEB_PROCESSOR

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "constants.h"
#include "web_cards.h"

// Card processor
String processor(const String& var);

#endif