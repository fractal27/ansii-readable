/*  see `enum ansii_type` in header */
#include "ansii.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "log.h"

enum {
    TRIPLET_N = 3,
    QUINTET_N = 5,
    ANSII_FG_START                = 30,
    ANSII_BG_START                = 40,
    ANSII_FONTATTR_RESET_START    = 20,
    ANSII_FONTATTR_BOLD           = 1u,
    ANSII_FONTATTR_FAINT          = 2u,
    ANSII_FONTATTR_ITALIC         = 3u,
    ANSII_FONTATTR_UNDERLINE      = 4u,
    ANSII_FONTATTR_BLINK          = 5u,
    ANSII_FONTATTR_INVERSE        = 6u,
    ANSII_FONTATTR_INVISIBLE      = 7u,
    ANSII_FONTATTR_STRIKETHROUGH  = 8u,
};

struct ansii_t {             
       char prefix_ch;
       union {
         unsigned int single;
         unsigned int pair[2];
         unsigned int triplet[TRIPLET_N];
         unsigned int quintet[QUINTET_N];
       } value;
       char suffix_ch;
       enum {
         VALUE_SINGLE = 0,
         VALUE_PAIR,
         VALUE_TRIPLET,
         VALUE_QUINTET
       } value_type;
       bool valid;
       bool no_bracket;
};

struct hash {
       unsigned long hash;
       bool computed;
};


inline unsigned long
hash(char str[], struct hash* to_compute)
{
#define HASH_BASE 5381
#define LOG_2_32  5
       if(to_compute && to_compute->computed){
              log_verbose("[already cached] `%lx`\n",to_compute->hash);
              return to_compute->hash;
       }
       unsigned char* ustr = (unsigned char*)str;
       unsigned long hash = HASH_BASE;
       int c = *ustr++;

       do {
         hash = ((hash << LOG_2_32) + hash) + c; // hash * 33 + c 
       } while ((c = *ustr++));

       if(to_compute) {
              to_compute->computed = true;
              to_compute->hash = hash;
              log_verbose("[just computed] `%lx`\n",hash);
       }
       return hash;
}

struct ansii_t 
erase_to_ansii(char* key, struct hash* phash){
       unsigned long key_hash = hash(key,phash);
       static struct hash h_curstoend, h_curstobeg, h_entire_scr, h_slines, h_curstoendline, h_starttocurs, h_entline;             

       if (key_hash == hash("from-cursor-to-end",&h_curstoend))          return (struct ansii_t){.value.single=0,.suffix_ch='J',.valid=true};
       if (key_hash == hash("from-cursor-to-beginning",&h_curstobeg))    return (struct ansii_t){.value.single=1,.suffix_ch='J',.valid=true};
       if (key_hash == hash("entire-screen",&h_entire_scr))              return (struct ansii_t){.value.single=2,.suffix_ch='J',.valid=true};
       if (key_hash == hash("saved-lines",&h_slines))                    return (struct ansii_t){.value.single=3,.suffix_ch='J',.valid=true};
       if (key_hash == hash("from-cursor-to-endline",&h_curstoendline))  return (struct ansii_t){.value.single=0,.suffix_ch='K',.valid=true};
       if (key_hash == hash("from-start-line-to-cursor",&h_starttocurs)) return (struct ansii_t){.value.single=1,.suffix_ch='K',.valid=true};
       if (key_hash == hash("entire-line",&h_entline))                   return (struct ansii_t){.value.single=2,.suffix_ch='K',.valid=true};
       log_error("Invalid value: '%s'\n",key);
       return (struct ansii_t){.valid=false};
}


struct ansii_t
cursor_to_ansii(char* key, struct hash* phash){
       unsigned long key_hash = hash(key,phash);
       static bool hashes_initialized = false;
#define VISIBILITY 25
#define REQ_POS    6

       static struct hash h_visible, h_invisible, h_to_origin, h_one_up, h_save, h_restore, h_request_position;
       unsigned int pair_to_read[2];
       unsigned int value_to_read = 0;

       if(key_hash == hash("invisible",&h_invisible))        return (struct ansii_t){'?',.value.single=VISIBILITY,.suffix_ch='l',.valid=true}; // ?25h
       else if(key_hash == hash("visible",&h_visible))       return (struct ansii_t){'?',.value.single=VISIBILITY,.suffix_ch='h',.valid=true};  // ?25l
       else if(key_hash == hash("restore",&h_restore))       return (struct ansii_t){.suffix_ch='u',.valid=true};  // ?25l
       else if(key_hash == hash("to-origin",&h_to_origin))   return (struct ansii_t){.suffix_ch='H',.valid=true};
       else if(key_hash == hash("one-up",&h_one_up))         return (struct ansii_t){.suffix_ch='M',.valid=true,.no_bracket=true};
       else if(key_hash == hash("save",&h_save))             return (struct ansii_t){.suffix_ch='s',.valid=true};
       else if(key_hash == hash("restore",&h_restore))       return (struct ansii_t){.suffix_ch='u',.valid=true};
       else if(key_hash == hash("request-position",&h_request_position)) 
              return (struct ansii_t){.value.single=REQ_POS,.suffix_ch='n',.valid=true};
       // tatic analisys
       struct ansii_t result = {};
       if(sscanf(key,"move-to-%u-%u",&pair_to_read[0],&pair_to_read[1])){
         result.suffix_ch = 'H';
         result.value_type = VALUE_PAIR;
         result.valid = true;
         if(memmove(result.value.pair,pair_to_read,2)){
           return result;
         }
         log_error("Memmove failed. pair not copied over");
       } else if(sscanf(key,"move-%u-up",&value_to_read)){    result.suffix_ch='A';
       } else if(sscanf(key,"move-%u-down",&value_to_read)){  result.suffix_ch='B';
       } else if(sscanf(key,"move-%u-right",&value_to_read)){ result.suffix_ch='C';
       } else if(sscanf(key,"move-%u-left",&value_to_read)){  result.suffix_ch='D';
       } else if(sscanf(key,"move-to-beginning-%u-lines-down",&value_to_read)){ result.suffix_ch='E';
       } else if(sscanf(key,"move-to-beginning-%u-lines-up",&value_to_read)){   result.suffix_ch='F';
       } else if(sscanf(key,"move-to-col-%u",&value_to_read)){                  result.suffix_ch='G';
       } else {
              log_error("Invalid value: '%s'\n",key);
              return (struct ansii_t){.valid=false};
       }
       // if here it means, that you came here from
       // } else if(sscanf...
       result.value.single = value_to_read;
       return result;
#undef VISIBILITY
}

struct ansii_t
altbuf_to_ansii(char* key, struct hash* phash){
       unsigned long key_hash = hash(key,phash);
#define ALTBUF_VAL   1049
       static struct hash h_enable, h_disable;
       struct ansii_t result = {.prefix_ch='?',.value.single=ALTBUF_VAL,.valid=true};

       if (key_hash == hash("enable",&h_enable)){
              result.suffix_ch='h';
       } else if (key_hash == hash("disable",&h_disable)){
              result.suffix_ch = 'l';
       } else {
              result.valid = false;
              log_error("Invalid value: '%s'\n",key);
       }
       return result;
#undef ALTBUF_VAL
}

struct ansii_t
scattr_to_ansii(char* key, struct hash* phash){
       unsigned long key_hash = hash(key,phash);
       static bool hashes_initialized = false;

       static struct hash h_screen_40_x_25_monochrome_text, h_screen_40_x_25_color_text, h_screen_80_x_25_monochrome_text, h_screen_80_x_25_color_text, 
                          h_screen_320_x_200_4_color_graphics, h_screen_320_x_200_monochrome_graphics, h_screen_640_x_200_monochrome_graphics, h_screen_linewrap, 
                          h_screen_320_x_200_color_graphics, h_screen_640_x_350_2_color_graphics, h_screen_640_x_350_16_color_graphics, h_screen_640_x_480_2_color_graphics, 
                          h_screen_640_x_480_16_color_graphics, h_screen_320_x_200_256_color_graphics, h_screen_restore, h_screen_save;
#define ANSII_SCREEN_RESTORE 47
#define ANSII_SCREEN_SAVE    47

       log_info("key_hash: %lu, should be %lu\n", key_hash, h_screen_40_x_25_monochrome_text);
       struct ansii_t result = {.prefix_ch = '=', .suffix_ch = 'h', .valid=true};
       if (key_hash == hash("40x25 monochrome text",&h_screen_40_x_25_monochrome_text))                  result.value.single=1; // 40 x 25 color 
       else if (key_hash == hash("40x25 color text",&h_screen_40_x_25_color_text))                       result.value.single=2; // 80 x 25 monochrome 
       else if (key_hash == hash("80x25 monochrome text",&h_screen_80_x_25_monochrome_text))             result.value.single=3; // 80 x 25 color 
       else if (key_hash == hash("80x25 color text",&h_screen_80_x_25_color_text))                       result.value.single=4; // 320 x 200 4-color 
       else if (key_hash == hash("320x200 4 color graphics",&h_screen_320_x_200_4_color_graphics))       result.value.single=5; // 320 x 200 monochrome
       else if (key_hash == hash("320x200 monochrome graphics",&h_screen_320_x_200_monochrome_graphics)) result.value.single=6; // 640 x 200 monochrome
       else if (key_hash == hash("640x200 monochrome graphics",&h_screen_640_x_200_monochrome_graphics)) result.value.single=7; // linewrap
       else if (key_hash == hash("linewrap",&h_screen_linewrap))                                         result.value.single=13; //320 x 200 color
       else if (key_hash == hash("320x200 color graphics",&h_screen_320_x_200_color_graphics))           result.value.single=14; //640 x 200 color 16col
       else if (key_hash == hash("640x350 2 color graphics",&h_screen_640_x_350_2_color_graphics))       result.value.single=15; //640 x 350 monochrome 2col
       else if (key_hash == hash("640x350 16 color graphics",&h_screen_640_x_350_16_color_graphics))     result.value.single=16; //640 x 350 color 16c
       else if (key_hash == hash("640x480 2 color graphics",&h_screen_640_x_480_2_color_graphics))       result.value.single=17; //640 x 480 monochrome (2-color graphics))
       else if (key_hash == hash("640x480 16 color graphics",&h_screen_640_x_480_16_color_graphics))     result.value.single=18; //640 x 480 color (16-color_graphics))
       else if (key_hash == hash("320x200 256 color graphics",&h_screen_320_x_200_256_color_graphics))   result.value.single=19; //320 x 200 color (256-color_graphics))
       else if (key_hash == hash("restore",&h_screen_restore)){                      
              result.prefix_ch='?';
              result.value.single=ANSII_SCREEN_RESTORE;
              result.suffix_ch='l'; //restore
       } else if (key_hash == hash("save",&h_screen_save)){
              result.prefix_ch='?';
              result.value.single=ANSII_SCREEN_SAVE;
       } else {
              log_error("Invalid value: '%s'\n",key);
              return (struct ansii_t){.valid=false};
       }
       return result; 
}



unsigned int
fntattr_to_ansii(char* font_attr,struct hash* phash){
       unsigned long key_hash = hash(font_attr,phash);
       static bool hashes_initialized = false;
       static struct hash h_fontattr_bold, h_fontattr_faint, h_fontattr_italic, h_fontattr_underline, h_fontattr_blink, h_fontattr_inverse, h_fontattr_invisible, h_fontattr_strikethrough;
       unsigned int attr_num = 0;

       if (key_hash == hash("bold",&h_fontattr_bold)){                          attr_num = ANSII_FONTATTR_BOLD;
       } else if (key_hash == hash("faint",&h_fontattr_faint)){                 attr_num = ANSII_FONTATTR_FAINT;
       } else if (key_hash == hash("italic",&h_fontattr_italic)){               attr_num = ANSII_FONTATTR_ITALIC;
       } else if (key_hash == hash("underline",&h_fontattr_underline)){         attr_num = ANSII_FONTATTR_UNDERLINE;
       } else if (key_hash == hash("blink",&h_fontattr_blink)){                 attr_num = ANSII_FONTATTR_BLINK;
       } else if (key_hash == hash("inverse",&h_fontattr_inverse)){             attr_num = ANSII_FONTATTR_INVERSE;
       } else if (key_hash == hash("invisible",&h_fontattr_invisible)){         attr_num = ANSII_FONTATTR_INVISIBLE;
       } else if (key_hash == hash("strikethrough",&h_fontattr_strikethrough)){ attr_num = ANSII_FONTATTR_STRIKETHROUGH;
       } else {
              log_error("Invalid font attribute: `%s`: (cached: %lx, actual: %lx)\n", font_attr,phash->hash,hash(font_attr,NULL));
       }
       log_info("(got) attribute: %u\n",attr_num);
       return attr_num;
}
struct ansii_t
color_to_ansii(char* color, struct hash* phash) {
       unsigned long key_hash = hash(color,phash);
       static bool hashes_initialized = false;
       static struct hash h_color_black, h_color_red, h_color_green, h_color_yellow, h_color_blue, h_color_magenta, h_color_cyan, h_color_white;
       unsigned int r=0,g=0,b=0,id=0;
       struct ansii_t result = (struct ansii_t){.value_type=VALUE_SINGLE,.suffix_ch='m',.valid=true};

       if (key_hash == hash("black",&h_color_black)){           result.value.single=0;
       } else if (key_hash == hash("red",&h_color_red)){        result.value.single=1;
       } else if (key_hash == hash("green",&h_color_green)){    result.value.single=2;
       } else if (key_hash == hash("yellow",&h_color_yellow)){  result.value.single=3;
       } else if (key_hash == hash("blue",&h_color_blue)){      result.value.single=4;
       } else if (key_hash == hash("magenta",&h_color_magenta)){result.value.single=5;
       } else if (key_hash == hash("cyan",&h_color_cyan)){      result.value.single=6;
       } else if (key_hash == hash("white",&h_color_white)){    result.value.single=7;
       } else if(sscanf(color,"id-%u",&id) == 1){
              result.value.triplet[0] = 8;
              result.value.triplet[1] = 5;
              result.value.triplet[2] = id;
              result.value_type = VALUE_TRIPLET;
       } else if(sscanf(color,"%u-%u-%u",&r,&g,&b) == 3){
              result.value.quintet[0] = 8;
              result.value.quintet[1] = 2;
              result.value.quintet[2] = r;
              result.value.quintet[3] = g;
              result.value.quintet[4] = b;
              result.value_type = VALUE_QUINTET;
       } else {
              result.valid = false;
              log_error("invalid color: %s.\n",color);
       }
       return result;
}

struct ansii_t
parse_ansii_type(enum ansii_types ansii_type, char* value, struct hash* valhash){
       struct ansii_t ansii_r = {.value_type = VALUE_SINGLE,.valid=true};
       switch(ansii_type){
         case BG:
           ansii_r = color_to_ansii(value,valhash);
           if(ansii_r.value_type == VALUE_SINGLE){
                  ansii_r.value.single += ANSII_BG_START;
           } else if (ansii_r.value_type == VALUE_QUINTET){ // rgb values
                  ansii_r.value.quintet[0] += ANSII_BG_START;
           }
           break;
         case FG:
           ansii_r = color_to_ansii(value,valhash);
           if(ansii_r.value_type == VALUE_SINGLE){
                  ansii_r.value.single += ANSII_FG_START;
           } else if (ansii_r.value_type == VALUE_QUINTET){ // rgb values
                  ansii_r.value.quintet[0] += ANSII_FG_START;
           }
           break;
         case FONT_SET:
           ansii_r.value.single = fntattr_to_ansii(value,valhash);
           ansii_r.suffix_ch = 'm';
           ansii_r.valid = true;
           break;
         case FONT_RESET:
           ansii_r.value.single = ANSII_FONTATTR_RESET_START + fntattr_to_ansii(value,valhash);
           ansii_r.valid = true;
           ansii_r.suffix_ch = 'm';
           break;
           // more complicated cases that, aren't just based on value.
         case SCREEN: return scattr_to_ansii(value,valhash);
         case CURSOR: return cursor_to_ansii(value,valhash);
         case ALTBUF: return altbuf_to_ansii(value,valhash);
         case ERASE: return erase_to_ansii(value,valhash);
         case RESET:
           ansii_r.value.single = 0;
           break;
         case NONE:
           ansii_r.valid = false;
       }
       return ansii_r;
}
int 
ansii_transform(FILE* from, FILE* to){
       // this assumes file `from` is readable, and `to`
       // is writable, and not equal. Make sure of this before calling this function.
#define BUFSIZE 1024
#define STROUT_LEN 8
       char buf[BUFSIZE];
       char* save_ptr_vkey = NULL;

       enum ansii_types ansii_type = NONE;
       unsigned int read = 0;
       struct ansii_t ansii_result;
       char* token_complete = NULL;
       char* end = NULL;
       char string_to_output[STROUT_LEN];
       static bool initialized_hashes = false;
       static struct hash h_bg = {},
                          h_fg = {},
                          h_font_set = {},
                          h_font_reset = {},
                          h_screen = {},
                          h_cursor = {},
                          h_altbuf = {},
                          h_erase = {},
                          h_reset = {};
       unsigned int key_hash = 0;
       struct hash vhash = {};


       while((read = fread(buf,sizeof(char),BUFSIZE,from))){
         // char* token_complete = strtok_r(buf,"{}",&save_ptr_buf);
         log_verbose("buffer read %u characters\n",read);
         token_complete = buf;
         while((token_complete = strstr(token_complete, "{{")) != NULL){
           token_complete += 2;
           end = strstr(token_complete,"}}");
           if(!end){
             break;
           }
           *end = 0;
           char* key = strtok_r(token_complete,":",&save_ptr_vkey);
           // recompute the hash for new key
           log_verbose("checking key");
           key_hash = hash(key,NULL);

           if(key_hash == hash("bg",&h_bg)){                     ansii_type = BG;
           } else if (key_hash == hash("fg",&h_fg)){             ansii_type = FG;
           } else if (key_hash == hash("font-set",&h_font_set)){ ansii_type = FONT_SET;
           } else if (key_hash == hash("font-reset",&h_font_reset)){
                                                                 ansii_type = FONT_RESET; 
           } else if (key_hash == hash("screen",&h_screen)){     ansii_type = SCREEN; 
           } else if (key_hash == hash("cursor",&h_cursor)){     ansii_type = CURSOR; 
           } else if (key_hash == hash("altbuf",&h_altbuf)){     ansii_type = ALTBUF;
           } else if (key_hash == hash("erase",&h_erase)){       ansii_type = ERASE;
           } else if (key_hash == hash("reset",&h_reset)){       ansii_type = RESET;
           } else {
                  ansii_type = NONE;
                  log_error("ANSII Key not valid: '%s'\n",key);
           }
           log_verbose("checking key");
           char* value = strtok_r(NULL,":",&save_ptr_vkey);
           vhash.computed = false;
           ansii_result = parse_ansii_type(ansii_type,value,&vhash);

           if(ansii_result.valid){
             switch(ansii_result.value_type){
                case VALUE_SINGLE:
                  if(ansii_result.prefix_ch){
                    fprintf(to,"\e[%c%d%c", ansii_result.prefix_ch,
                        ansii_result.value.single,
                        ansii_result.suffix_ch);
                  } else {
                    fprintf(to,"\e[%d%c",
                        ansii_result.value.single,
                        ansii_result.suffix_ch);
                  }
                  break;

                case VALUE_PAIR:
                  if(ansii_result.prefix_ch){
                    fprintf(to,"\e[%c%u;%u%c", ansii_result.prefix_ch, 
                        ansii_result.value.pair[0], ansii_result.value.pair[1],
                        ansii_result.suffix_ch);
                  } else {
                    fprintf(to,"\e[%u;%u%c",
                        ansii_result.value.pair[0], ansii_result.value.pair[1],
                        ansii_result.suffix_ch);
                  }
                  break;
                case VALUE_TRIPLET:
                  if(ansii_result.prefix_ch){
                    fprintf(to,"\e[%c%u;%u;%u%c", ansii_result.prefix_ch, 
                        ansii_result.value.triplet[0], ansii_result.value.triplet[1], ansii_result.value.triplet[2], 
                        ansii_result.suffix_ch);
                  } else {
                    fprintf(to,"\e[%u;%u;%u%c",
                        ansii_result.value.triplet[0], ansii_result.value.triplet[1], ansii_result.value.triplet[2],
                        ansii_result.suffix_ch);
                  }
                  break;
                // NTS: currently only used for fg/bg RGB values
                case VALUE_QUINTET: 
                  if(ansii_result.prefix_ch){
                    fprintf(to,"\e[%c%u;%u;%u;%u;%u%c", ansii_result.prefix_ch, 
                        ansii_result.value.quintet[0], ansii_result.value.quintet[1], ansii_result.value.quintet[2], ansii_result.value.quintet[3], ansii_result.value.quintet[4],
                        ansii_result.suffix_ch);
                  } else {
                    fprintf(to,"\e[%u;%u;%u;%u;%u%c",
                        ansii_result.value.triplet[0], ansii_result.value.triplet[1], ansii_result.value.triplet[2],ansii_result.value.quintet[3], ansii_result.value.quintet[4],
                        ansii_result.suffix_ch);
                  }
                  break;

                default:
                  break;
             }
           }
           // go until { and take the token
           token_complete = end + 2;

           end = strstr(token_complete,"{{");
           if(!end){
             break;
           }
           *end = '\0';
           fprintf(to,"%s",token_complete); // send to file the string until {{
           *end = '{';                 // reset the character
           fflush(to);
         }
       }
       return 0;

#undef BUFSIZE
#undef STROUT_LEN
}


