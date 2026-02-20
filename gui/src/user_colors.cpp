#include <wx/colour.h>
#include <wx/hashmap.h>

#include "user_colors.h"
#include "color_types.h"
#include "color_handler.h"
#include "s52plib.h"

ColorScheme global_color_scheme = GLOBAL_COLOR_SCHEME_DAY;

namespace user_colors {

ColorScheme GetColorScheme() { return global_color_scheme; }

static const char *const usercolors[] = {
    //======================================================================
    // Table:DAY - Bright daylight color scheme (full visibility mode)
    //======================================================================
    "Table:DAY",

    // Standard palette colors - general purpose UI elements
    "GREEN1;120;255;120;", "GREEN2; 45;150; 45;", "GREEN3;200;220;200;",
    "GREEN4;  0;255;  0;", "GREEN5;170;254;  0;", "BLUE1; 170;170;255;",
    "BLUE2;  45; 45;170;", "BLUE3;   0;  0;255;", "GREY1; 200;200;200;",
    "GREY2; 230;230;230;", "RED1;  220;200;200;", "YELO1; 243;229; 47;",
    "YELO2; 128; 80;  0;", "TEAL1;   0;128;128;",

    // Basic UI colors
    "UBLCK;   0;  0;  0;",  // Universal black for text/lines
    "UWHIT; 255;255;255;",  // Universal white for backgrounds
    "URED;  255;  0;  0;",  // Own vessel color, AIS targets, predictor lines
    "UGREN;   0;255;  0;",  // Universal green for general purpose green
    "COMPT; 245;247;244",   // Compass rose background/details

// Dialog system colors
#ifdef __WXOSX__
    "DILG0; 255;255;255;",  // Dialog window background (macOS)
#else
    "DILG0; 238;239;242;",  // Dialog window background (other platforms)
#endif
    "DILG1; 212;208;200;",  // Background color for selected items
    "DILG2; 255;255;255;",  // Control backgrounds for text boxes and input
                            // fields
    "DILG3;   0;  0;  0;",  // Dialog text color in dialogs and controls
    /**
     * Text color optimized for progressively darker backgrounds (pairs with
     * DILG0). Gets progressively lighter as background darkens to maintain
     * contrast. Ideal for tooltips, overlays, and any text over DILG0
     * background. */
    "DILG4;   0;  0;  0;",
    "UITX1;   0;  0;  0;",  // Menu text color

    // Chart and information display colors
    "CHGRF; 163; 180; 183;",  // Chart gray foreground (grid lines, secondary
                              // text)
    "CHYLW; 244; 218;  72;",  // Chart yellow (AIS name invalid, warnings)
    "CHWHT; 212; 234; 238;",  // Chart white (AIS outlines, contrast elements)

    // Information status colors
    "UINFM; 197;  69; 195;",  // Magenta - special indicators, chart magenta
                              // features
    "UINFG; 104; 228;  86;",  // Green - status indicators, tide/current
                              // graphics
    "UINFR; 241;  84; 105;",  // Red - alerts, errors, danger markers
    "UINFF; 125; 137; 140;",  // Default foreground - general UI elements
    "SHIPS;   7;   7;   7;",  // Other vessels/AIS target fills

    // Route and navigation colors
    "UDKRD; 124; 16;  0;",  // Dark red variant - reduced visibility alternative
                            // to URED
    "UARTE; 200;  0;  0;",  // Active route color (bright red in day mode)

    // Chart data and measurement colors
    "NODTA; 163; 180; 183;",  // No data available areas
    "CHBLK;   7;   7;   7;",  // Chart black - text, lines, piano keys
    "SNDG1; 125; 137; 140;",  // Sounding text (depth numbers) - primary
    "SNDG2;   7;   7;   7;",  // Sounding text (depth numbers) - secondary
    "SCLBR; 235; 125;  54;",  // Scale bar markings and text

    // UI framework colors
    "UIBDR; 125; 137; 140;",  // UI borders, status bar background
    "UIBCK; 212; 234; 238;",  // Highlight backgrounds, info windows
    "UINFB;  58; 120; 240;",  // Information blue - active/selected states, tide
                              // markers
    "UINFD;   7;   7;   7;",  // Information dark - borders, inactive elements
    "UINFO; 235; 125;  54;",  // Information orange - warnings, highlights

    // Route planning colors
    "PLRTE; 220;  64;  37;",  // Planned route color (not yet active)
    "CHMGD; 197; 69; 195;",  // Chart magenta - special chart features, AIS MMSI
                             // text

    // Dashboard instrument colors
    "DASHB; 255;255;255;",  // Dashboard instrument background
    "DASHL; 175;175;175;",  // Dashboard instrument labels and graduations
    "DASHF;  50; 50; 50;",  // Dashboard foreground text and indicators
    "DASHR; 200;  0;  0;",  // Dashboard red indicators (alarms, danger zones)
    "DASHG;   0;200;  0;",  // Dashboard green indicators (normal status)
    "DASHN; 200;120;  0;",  // Dashboard needle/pointer color
    "DASH1; 204;204;255;",  // Dashboard graphic elements - primary
    "DASH2; 122;131;172;",  // Dashboard graphic elements - secondary
    "COMP1; 211;211;211;",  // Compass window background

    // Window and canvas elements
    "GREY3;  40; 40; 40;",     // MUI toolbar background
    "BLUE4; 100;100;200;",     // Canvas focus indicator bar
    "VIO01; 171; 33;141;",     // Violet - vector chart special elements
    "VIO02; 209;115;213;",     // Violet variant - vector chart features
    "BLUEBACK; 212;234;238;",  // Deep water background color for basemap
    "LANDBACK; 201;185;122;",  // Land mass background color for basemap

    //======================================================================
    // Table:DUSK - Reduced brightness for twilight conditions
    // Colors defined above are automatically dimmed for dusk visibility
    //======================================================================
    "Table:DUSK", "GREEN1; 60;128; 60;", "GREEN2; 22; 75; 22;",
    "GREEN3; 80;100; 80;", "GREEN4;  0;128;  0;", "BLUE1;  80; 80;160;",
    "BLUE2;  30; 30;120;", "BLUE3;   0;  0;128;", "GREY1; 100;100;100;",
    "GREY2; 128;128;128;", "RED1;  150;100;100;", "UBLCK;   0;  0;  0;",
    "UWHIT; 255;255;255;", "URED;  120; 54; 11;", "UGREN;  35;110; 20;",
    "YELO1; 120;115; 24;", "YELO2;  64; 40;  0;", "TEAL1;   0; 64; 64;",
    "GREEN5; 85;128; 0;", "COMPT; 124;126;121",

    "CHGRF;  41; 46; 46;", "UINFM;  58; 20; 57;", "UINFG;  35; 76; 29;",
    "UINFF;  41; 46; 46;", "UINFR;  80; 28; 35;", "SHIPS;  71; 78; 79;",
    "CHYLW;  81; 73; 24;", "CHWHT;  71; 78; 79;",

    "DILG0; 110;110;110;",  // Dialog Background
    "DILG1; 110;110;110;",  // Dialog Background
    "DILG2;   0;  0;  0;",  // Control Background
    "DILG3; 130;130;130;",  // Text
    "DILG4;   0;  0;  0;",
    "UITX1;  41; 46; 46;",  // Menu text color
    "UDKRD;  80;  0;  0;",  // Dark red variant - reduced visibility alternative
                            // to URED
    "UARTE;  64; 64; 64;",  // Active route color (grey for dusk/night modes)

    "NODTA;  41;  46;  46;", "CHBLK;  54;  60;  61;", "SNDG1;  41;  46;  46;",
    "SNDG2;  71;  78;  79;", "SCLBR;  75;  38;  19;", "UIBDR;  54;  60;  61;",
    "UINFB;  19;  40;  80;", "UINFD;  71;  78;  79;", "UINFO;  75;  38;  19;",
    "PLRTE;  73;  21;  12;", "CHMGD; 74; 58; 81;", "UIBCK; 7; 7; 7;",

    "DASHB;  77; 77; 77;",  // Dashboard Instr background
    "DASHL;  54; 54; 54;",  // Dashboard Instr Label
    "DASHF;   0;  0;  0;",  // Dashboard Foreground
    "DASHR;  58; 21; 21;",  // Dashboard Red
    "DASHG;  21; 58; 21;",  // Dashboard Green
    "DASHN; 100; 50;  0;",  // Dashboard Needle
    "DASH1;  76; 76;113;",  // Dashboard Illustrations
    "DASH2;  48; 52; 72;",  // Dashboard Illustrations
    "COMP1; 107;107;107;",  // Compass Window Background

    "GREY3;  20; 20; 20;",  // MUIBar/TB background
    "BLUE4;  80; 80;160;",  // Canvas Focus Bar
    "VIO01; 128; 25;108;", "VIO02; 171; 33;141;", "BLUEBACK; 186;213;235;",
    "LANDBACK; 201;185;122;",

    //======================================================================
    // Table:NIGHT - Dark adapted colors preserving night vision
    // Colors are further dimmed and shifted toward red spectrum
    //======================================================================
    "Table:NIGHT", "GREEN1; 30; 80; 30;", "GREEN2; 15; 60; 15;",
    "GREEN3; 12; 23;  9;", "GREEN4;  0; 64;  0;", "BLUE1;  60; 60;100;",
    "BLUE2;  22; 22; 85;", "BLUE3;   0;  0; 40;", "GREY1;  48; 48; 48;",
    "GREY2;  32; 32; 32;", "RED1;  100; 50; 50;", "UWHIT; 255;255;255;",
    "UBLCK;   0;  0;  0;", "URED;   60; 27;  5;", "UGREN;  17; 55; 10;",
    "YELO1;  60; 65; 12;", "YELO2;  32; 20;  0;", "TEAL1;   0; 32; 32;",
    "GREEN5; 44; 64; 0;", "COMPT;  48; 49; 51",
    "DILG0;  80; 80; 80;",     // Dialog Background
    "DILG1;  80; 80; 80;",     // Dialog Background
    "DILG2;   0;  0;  0;",     // Control Background
    "DILG3;  180; 180; 180;",  // Text
    "DILG4; 220;220;220;",
    "UITX1;  31; 34; 35;",  // Menu text color
    "UDKRD;  50;  0;  0;",  // Dark red variant - reduced visibility alternative
                            // to URED
    "UARTE;  64; 64; 64;",  // Active route color (grey for dusk/night modes)

    "CHGRF;  16; 18; 18;", "UINFM;  52; 18; 52;", "UINFG;  22; 24;  7;",
    "UINFF;  31; 34; 35;", "UINFR;  59; 17; 10;", "SHIPS;  37; 41; 41;",
    "CHYLW;  31; 33; 10;", "CHWHT;  37; 41; 41;",

    "NODTA;   7;   7;   7;", "CHBLK;  31;  34;  35;", "SNDG1;  31;  34;  35;",
    "SNDG2;  43;  48;  48;", "SCLBR;  52;  28;  12;", "UIBDR;  31;  34;  35;",
    "UINFB;  21;  29;  69;", "UINFD;  43;  48;  58;", "UINFO;  52;  28;  12;",
    "PLRTE;  66;  19;  11;", "CHMGD; 52; 18; 52;", "UIBCK; 7; 7; 7;",

    "DASHB;   0;  0;  0;",  // Dashboard Instr background
    "DASHL;  20; 20; 20;",  // Dashboard Instr Label
    "DASHF;  64; 64; 64;",  // Dashboard Foreground
    "DASHR;  70; 15; 15;",  // Dashboard Red
    "DASHG;  15; 70; 15;",  // Dashboard Green
    "DASHN;  17; 80; 56;",  // Dashboard Needle
    "DASH1;  48; 52; 72;",  // Dashboard Illustrations
    "DASH2;  36; 36; 53;",  // Dashboard Illustrations
    "COMP1;  24; 24; 24;",  // Compass Window Background

    "GREY3;  10; 10; 10;",  // MUIBar/TB background
    "BLUE4;  70; 70;140;",  // Canvas Focus Bar
    "VIO01;  85; 16; 72;", "VIO02; 128; 25;108;", "BLUEBACK; 186;213;235;",
    "LANDBACK; 201;185;122;",

    "*****"};

static wxArrayPtrVoid *UserColorTableArray = 0;
static wxArrayPtrVoid *UserColourHashTableArray;

static int get_static_line(char *d, const char *const *p, int index, int n) {
  if (!strcmp(p[index], "*****")) return 0;

  strncpy(d, p[index], n);
  return strlen(d);
}

void Initialize() {
  const char *const *p = usercolors;
  char buf[81];
  int index = 0;
  char TableName[20];
  colTable *ctp;
  colTable *ct;
  int R, G, B;

  UserColorTableArray = new wxArrayPtrVoid;
  UserColourHashTableArray = new wxArrayPtrVoid;

  //    Create 3 color table entries
  ct = new colTable;
  ct->tableName = new wxString("DAY");
  ct->color = new wxArrayPtrVoid;
  UserColorTableArray->Add((void *)ct);

  ct = new colTable;
  ct->tableName = new wxString("DUSK");
  ct->color = new wxArrayPtrVoid;
  UserColorTableArray->Add((void *)ct);

  ct = new colTable;
  ct->tableName = new wxString("NIGHT");
  ct->color = new wxArrayPtrVoid;
  UserColorTableArray->Add((void *)ct);

  while ((get_static_line(buf, p, index, sizeof(buf) - 1))) {
    if (!strncmp(buf, "Table", 5)) {
      sscanf(buf, "Table:%s", TableName);

      for (unsigned int it = 0; it < UserColorTableArray->GetCount(); it++) {
        ctp = (colTable *)(UserColorTableArray->Item(it));
        if (!strcmp(TableName, ctp->tableName->mb_str())) {
          ct = ctp;
          break;
        }
      }

    } else {
      char name[21];
      int j = 0;
      while (buf[j] != ';' && j < 20) {
        name[j] = buf[j];
        j++;
      }
      name[j] = 0;

      S52color *c = new S52color;
      strcpy(c->colName, name);

      sscanf(&buf[j], ";%i;%i;%i", &R, &G, &B);
      c->R = (char)R;
      c->G = (char)G;
      c->B = (char)B;

      ct->color->Add(c);
    }

    index++;
  }

  //    Now create the Hash tables

  for (unsigned int its = 0; its < UserColorTableArray->GetCount(); its++) {
    wxColorHashMap *phash = new wxColorHashMap;
    UserColourHashTableArray->Add((void *)phash);

    colTable *ctp = (colTable *)(UserColorTableArray->Item(its));

    for (unsigned int ic = 0; ic < ctp->color->GetCount(); ic++) {
      S52color *c2 = (S52color *)(ctp->color->Item(ic));

      wxColour c(c2->R, c2->G, c2->B);
      wxString key(c2->colName, wxConvUTF8);
      (*phash)[key] = c;
    }
  }

  //    Establish a default hash table pointer
  //    in case a color is needed before ColorScheme is set
  pcurrent_user_color_hash =
      (wxColorHashMap *)UserColourHashTableArray->Item(0);
}
void DeInitialize() {
  if (!UserColorTableArray) return;
  for (unsigned i = 0; i < UserColorTableArray->GetCount(); i++) {
    colTable *ct = (colTable *)UserColorTableArray->Item(i);

    for (unsigned int j = 0; j < ct->color->GetCount(); j++) {
      S52color *c = (S52color *)ct->color->Item(j);
      delete c;  // color
    }

    delete ct->tableName;  // wxString
    delete ct->color;      // wxArrayPtrVoid

    delete ct;  // colTable
  }

  delete UserColorTableArray;

  for (unsigned i = 0; i < UserColourHashTableArray->GetCount(); i++) {
    wxColorHashMap *phash = (wxColorHashMap *)UserColourHashTableArray->Item(i);
    delete phash;
  }

  delete UserColourHashTableArray;
}

wxColorHashMap *GetMapByScheme(const std::string &scheme_name) {
  unsigned Usercolortable_index = 0;
  for (unsigned int i = 0; i < UserColorTableArray->GetCount(); i++) {
    colTable *ct = (colTable *)UserColorTableArray->Item(i);
    if (scheme_name == (*ct->tableName)) {
      return (wxColorHashMap *)UserColourHashTableArray->Item(i);
    }
  }
  return nullptr;
}

wxColor GetDimColor(wxColor c) {
  if ((global_color_scheme == GLOBAL_COLOR_SCHEME_DAY) ||
      (global_color_scheme == GLOBAL_COLOR_SCHEME_RGB))
    return c;

  float factor = 1.0;
  if (global_color_scheme == GLOBAL_COLOR_SCHEME_DUSK) factor = 0.5;
  if (global_color_scheme == GLOBAL_COLOR_SCHEME_NIGHT) factor = 0.25;

  wxImage::RGBValue rgb(c.Red(), c.Green(), c.Blue());
  wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
  hsv.value = hsv.value * factor;
  wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);

  return wxColor(nrgb.red, nrgb.green, nrgb.blue);
}

}  // namespace user_colors
