#ifndef gpsdatum_h
#define gpsdatum_h



  typedef struct GPS_SEllipse {
    const char*   name;
    double a;
    double invf;
  } GPS_OEllipse, *GPS_PEllipse;

  GPS_OEllipse GPS_Ellipse[]= {
    { "Airy 1830",               6377563.396, 299.3249646 },
    { "Airy 1830 Modified",      6377340.189, 299.3249646 },
    { "Australian National",     6378160.000, 298.25 },
    { "Bessel 1841 (Namibia)",   6377483.865, 299.1528128 },
    { "Bessel 1841",             6377397.155, 299.1528128 },
    { "Clarke 1866",             6378206.400, 294.9786982 },
    { "Clarke 1880",             6378249.145, 293.465 },
    { "Everest (India 1830)",    6377276.345, 300.8017 },
    { "Everest (Sabah Sarawak)", 6377298.556, 300.8017 },
    { "Everest (India 1956)",    6377301.243, 300.8017 },
    { "Everest (Malaysia 1969)", 6377295.664, 300.8017 },
    { "Everest (Malay & Sing)",  6377304.063, 300.8017 },
    { "Everest (Pakistan)",      6377309.613, 300.8017 },
    { "Modified Fischer 1960",   6378155.000, 298.3 },
    { "Helmert 1906",            6378200.000, 298.3 },
    { "Hough 1960",              6378270.000, 297.0 },
    { "Indonesian 1974",         6378160.000, 298.247 },
    { "International 1924",      6378388.000, 297.0 },
    { "Krassovsky 1940",         6378245.000, 298.3 },
    { "GRS67",                   6378160.000, 6356774.516 },
    { "GRS75",                   6378140.000, 6356755.288 },
    { "GRS80",                   6378137.000, 298.257222101 },
    { "S. American 1969",        6378160.000, 298.25 },
    { "WGS60",                   6378165.000, 298.3 },
    { "WGS66",                   6378145.000, 298.25 },
    { "WGS72",                   6378135.000, 298.26 },
    { "WGS84",                   6378137.000, 298.257223563 },
    { "Clarke 1880 (Benoit)",    6378300.789, 293.466 },
  };



  typedef struct GPS_SDatum {
    const char*   name;
    int    ellipse;
    double dx;
    double dy;
    double dz;
  } GPS_ODatum, *GPS_PDatum;

  GPS_ODatum GPS_Datum[]= {
    /* 000 */    { "Adindan",  		6,	-166,  	-15,	204 	},
    /* 001 */    { "AFG",     		18,	-43,   	-163,  	45 	},
    /* 002 */    { "Ain-El-Abd",  		17,	-150,  	-251,  	-2 	},
    /* 003 */    { "Alaska-NAD27",    	5,	-5,    	135,  	172 	},
    /* 004 */    { "Alaska-Canada",   	6,	-9,    	151,  	185 	},
    /* 005 */    { "Anna-1-Astro",    	2,	-491,  	-22,  	435 	},
    /* 006 */    { "ARC 1950 Mean",   	6,	-143,  	-90, 	-294 	},
    /* 007 */    { "ARC 1960 Mean",   	6,	-160,  	-8,  	-300 	},
    /* 008 */    { "Asc Island 58",  	17,	-207,  	107,   	52 	},
    /* 009 */    { "Astro B4",       	17,	114,  	-116,	-333 	},
    /* 010 */    { "Astro Beacon E", 	17,	145,  	75,  	-272 	},
    /* 011 */    { "Astro pos 71/4", 	17,	-320,  	550, 	-494 	},
    /* 012 */    { "Astro stn 52",   	17,	124,  	-234, 	-25 	},
    /* 013 */    { "Australia Geo 1984",  	2,	-134,  	-48,  	149 	},
    /* 014 */    { "Bahamas NAD27",   	6,	-4,    	154,  	178 	},
    /* 015 */    { "Bellevue IGN",   	17,	-127,  	-769, 	472 	},
    /* 016 */    { "Bermuda 1957",   	6,	-73,   	213,  	296 	},
    /* 017 */    { "Bukit Rimpah", 	 	4,	-384,  	664,  	-48 	},
    /* 018 */    { "Camp_Area_Astro",	17,     -104, 	-129,  	239 	},
    /* 019 */    { "Campo_Inchauspe",	17,     -148, 	136,    90 	},
    /* 020 */    { "Canada_Mean(NAD27)", 	5,  	-10,  	158,   	187 	},
    /* 021 */    { "Canal_Zone_(NAD27)", 	5,  	0,    	125,   	201 	},
    /* 022 */    { "Canton_Island_1966", 	17, 	298, 	-304,  	-375 	},
    /* 023 */    { "Cape",           	6,      -136, 	-108,  	-292 	},
    /* 024 */    { "Cape_Canaveral_mean", 	5, 	-2,    	150,   	181 	},
    /* 025 */    { "Carribean NAD27", 	5,     	-7,    	152,   	178 	},
    /* 026 */    { "Carthage",        	6,     	-263,  	6,     	431 	},
    /* 027 */    { "Cent America NAD27", 	5 , 	0,     	125,   	194 	},
    /* 028 */    { "Chatham 1971",   	17,     175,   	-38,   	113 	},
    /* 029 */    { "Chua Astro",     	17,     -134,  	229,   	-29 	},
    /* 030 */    { "Corrego Alegre", 	17,     -206,  	172,    -6 	},
    /* 031 */    { "Cuba NAD27",         	5,  	-9,    	152,   	178 	},
    /* 032 */    { "Cyprus",         	17,     -104, 	-101,  	-140 	},
    /* 033 */    { "Djakarta(Batavia)",  	4,  	-377,  	681,   	-50 	},
    /* 034 */    { "DOS 1968",          	17,     230,  	-199,  	-752 	},
    /* 035 */    { "Easter lsland 1967", 	17, 	211,   	147,   	111 	},
    /* 036 */    { "Egypt",             	17,  	-130, 	-117,  	-151 	},
    /* 037 */    { "European 1950",     	17,  	-87,  	-96,   	-120 	},
    /* 038 */    { "European 1950 mean",	17,  	-87,  	-98,   	-121 	},
    /* 039 */    { "European 1979 mean",	17,  	-86,  	-98,   	-119 	},
    /* 040 */    { "Finnish Nautical",  	17,  	-78, 	-231,   -97 	},
    /* 041 */    { "Gandajika Base",    	17,  	-133,	-321,   50 	},
    /* 042 */    { "Geodetic Datum 49", 	17,  	84,  	-22,    209 	},
    /* 043 */    { "Ghana",             	26,  	0,  	0,      0 	},
    /* 044 */    { "Greenland NAD27",    	5,  	11,  	114,    195 	},
    /* 045 */    { "Guam 1963",          	5,  	-100,	-248,   259 	},
    /* 046 */    { "Gunung Segara",      	4,  	-403,	684,    41 	},
    /* 047 */    { "Gunung Serindung 1962",	26, 	0,     	0,     	0 	},
    /* 048 */    { "GUX1 Astro",        	17,  	252, 	-209,   -751 	},
    /* 049 */    { "Herat North",       	17,  	-333,	-222,   114 	},
    /* 050 */    { "Hjorsey 1955",      	17,  	-73,  	46,     86 	},
    /* 051 */    { "Hong Kong 1963",    	17,  	-156, 	-271,  	-189 	},
    /* 052 */    { "Hu-Tzu-Shan",       	17,  	-634, 	-549,  	-201 	},
    /* 053 */    { "Indian",             	9,  	289,  	734,    257 	},
    /* 054 */    { "Iran",              	17,  	-117, 	-132,  	-164 	},
    /* 055 */    { "Ireland 1965",       	1,  	506,  	-122,   611 	},
    /* 056 */    { "ISTS 073 Astro 69", 	17,  	208,  	-435,  	-229 	},
    /* 057 */    { "Johnston Island 61",	17,  	191,   	-77,  	-204 	},
    /* 058 */    { "Kandawala",          	7,  	-97,   	787,    86 	},
    /* 059 */    { "Kerguelen Island",  	17,  	145,  	-187,   103 	},
    /* 060 */    { "Kertau 48",         	11,  	-11,   	851,    5 	},
    /* 061 */    { "L.C. 5 Astro",       	5,  	42,   	124,   	147 	},
    /* 062 */    { "La Reunion",        	17,  	94,  	-948, 	-1262 	},
    /* 063 */    { "Liberia 1964",       	6,  	-90,    40,    	88 	},
    /* 064 */    { "Luzon",              	5,  	-133,   -77,   	-51 	},
    /* 065 */    { "Mahe 1971",          	6,  	41,  	-220,  	-134 	},
    /* 066 */    { "Marco Astro",       	17,  	-289,  	-124,   60 	},
    /* 067 */    { "Masirah Is. Nahrwan", 	6, 	-247,  	-148,   369 	},
    /* 068 */    { "Massawa",            	4,   	639,   	405,    60 	},
    /* 069 */    { "Merchich",           	6,  	31,   	146,    47 	},
    /* 070 */    { "Mexico NAD27",       	5,  	-12,   	130,   	190 	},
    /* 071 */    { "Midway Astro 61",   	17,  	912,   	-58,  	1227 	},
    /* 072 */    { "Mindanao",           	5,  	-133,   -79,   	-72 	},
    /* 073 */    { "Minna",              	6,  	-92,   	-93,   	122 	},
    /* 074 */    { "Montjong Lowe",     	26,  	0,     	0,     	0 	},
    /* 075 */    { "Nahrwan",            	6,  	-231,  	-196,   482 	},
    /* 076 */    { "Naparima BWI",      	17,  	-2,   	374,   	172 	},
    /* 077 */    { "North America 83",  	21,  	0,     	0,     	0 	},
    /* 078 */    { "N. America 1927 mean",	5, 	-8,   	160,   	176 	},
    /* 079 */    { "Observatorio 1966", 	17,  	-425,  	-169,   81 	},
    /* 080 */    { "Old Egyptian",      	14,  	-130,   110,   	-13 	},
    /* 081 */    { "Old Hawaiian_mean",   	5, 	89,  	-279,  	-183 	},
    /* 082 */    { "Old Hawaiian Kauai", 	5,  	45,  	-290,  	-172 	},
    /* 083 */    { "Old Hawaiian Maui",  	5,  	65,  	-290,  	-190 	},
    /* 084 */    { "Old Hawaiian Oahu",  	5,  	56,  	-284,  	-181 	},
    /* 085 */    { "Oman",               	6,  	-346,   -1,   	224 	},
    /* 086 */    { "OSGB36",             	0,  	375,  	-111,   431 	},
    /* 087 */    { "Pico De Las Nieves",	17,  	-307,   -92,   	127 	},
    /* 088 */    { "Pitcairn Astro 67", 	17,  	185,   	165,    42 	},
    /* 089 */    { "S. Am. 1956 mean(P)",	17, 	-288,   175,  	-376 	},
    /* 090 */    { "S. Chilean 1963 (P)",	17, 	16,   	196,    93 	},
    /* 091 */    { "Puerto Rico",        	5,  	11,    	72,  	-101 	},
    /* 092 */    { "Pulkovo 1942",      	18,  	28,  	-130,   -95 	},
    /* 093 */    { "Qornoq",            	17,  	164,   	138,  	-189 	},
    /* 094 */    { "Quatar National",   	17,  	-128,  	-283,   22 	},
    /* 095 */    { "Rome 1940",         	17,  	-225,  	-65,    9 	},
    /* 096 */    { "S-42(Pulkovo1942)", 	18,  	28,  	-121,   -77 	},
    /* 097 */    { "S.E.Asia_(Indian)",  	7,  	173,   	750,   	264 	},
    /* 098 */    { "SAD-69/Brazil",     	22,  	-60,    -2,   	-41 	},
    /* 099 */    { "Santa Braz",        	17,  	-203,   141,    53 	},
    /* 100 */    { "Santo (DOS)",       	17,  	170,    42,    	84 	},
    /* 101 */    { "Sapper Hill 43",    	17,  	-355,   16,    	74 	},
    /* 102 */    { "Schwarzeck",         	3,  	616,    97,  	-251 	},
    /* 103 */    { "Sicily",            	17,  	-97,   	-88,  	-135 	},
    /* 104 */    { "Sierra Leone 1960", 	26,  	0,     	0,     	0 	},
    /* 105 */    { "S. Am. 1969 mean",  	22,  	-57,    1,   	-41 	},
    /* 106 */    { "South Asia",        	13,  	7,   	-10,   	-26 	},
    /* 107 */    { "Southeast Base",    	17,  	-499,  	-249,   314 	},
    /* 108 */    { "Southwest Base",    	17,  	-104,   167,   	-38 	},
    /* 109 */    { "Tananarive Obs 25", 	17,  	-189,  	-242,   -91 	},
    /* 110 */    { "Thai/Viet (Indian)", 	7,  	214,   	836,   	303 	},
    /* 111 */    { "Timbalai 1948",      	7,  	-689,   691,   	-45 	},
    /* 112 */    { "Tokyo mean",         	4,  	-128,   481,   	664 	},
    /* 113 */    { "Tristan Astro 1968",	17,  	-632,   438,  	-609 	},
    /* 114 */    { "United Arab Emirates",	6, 	-249, 	-156,   381 	},
    /* 115 */    { "Viti Levu 1916",     	6,  	51,  	391,   	-36 	},
    /* 116 */    { "Wake Eniwetok 60",  	15,  	101,   	52,   	-39 	},
    /* 117 */    { "WGS 72",            	25,  	0,    	0,     	5 	},
    /* 118 */    { "WGS 84",            	26,  	0,    	0,     	0 	},
    /* 119 */    { "Yacare",            	17,  	-155,  	171,    37 	},
    /* 120 */    { "Zanderij",          	17,  	-265,  	120,  	-358 	},
    /* 121 */    { "Sweden",          	4,  	424.3, 	-80.5, 	613.1 	},
    /* 122 */    { "GDA 94",		21,  	0, 	0, 	0 	},
    /* 123 */    { "CH-1903",		4,  	674, 	15, 	405 	},
    /* 124 */    { "Palestine 1923",        27,     -235,   -85,    264     },
    /* 125 */    { "ITM (Israeli New)",     21,     -48,     55,    -52     },
    { nullptr,                 	0,  	0,    	0,     	0 	}
  };


  typedef struct GPS_SDatum_Alias {
    const char* alias;
    const int datum;
  } GPS_ODatum_Alias, *GPS_PDatum_Alias;

  GPS_ODatum_Alias GPS_DatumAlias[] = {
    { "Australian GDA94", 122 },
    { "Australian Geocentric 1994 (GDA94)", 122},  /* Observed in Ozi */
    { "GDA94", 122 },
    { "GDA-94", 122 },
    { "CH1903", 123 },
    { "CH 1903", 123 },
    { "European 1950 (Spain and Portugal)", 38 },
    { "Geodetic Datum 1949", 42 },
    { "NAD27 Alaska", 3 },
    { "NAD27 Bahamas", 14 },
    { "NAD27 Canada", 4 },
    { "NAD27 Canal Zone", 21 },
    { "NAD27 Caribbean", 25 },
    { "NAD27 Central", 27 },
    { "NAD27 CONUS", 78 },
    { "NAD27 Cuba", 31 },
    { "NAD27 Greenland", 44 },
    { "NAD27 Mexico", 70 },
    { "NAD83", 77 },
    { "NAD 83", 77 },
    { "NAD-83", 77 },
    { "OSGB 36", 86 },
    { "OSGB-36", 86 },
    { "Wake-Eniwetok 1960", 116 },
    { "WGS72", 117 },
    { "WGS-72", 117 },
    { "WGS84", 118 },
    { "WGS-84", 118 },
    { "Israeli", 124 },
    { "D_Israel_new", 125 },
    { nullptr, -1 }
  };


  /* UK Ordnance Survey Nation Grid Map Codes */
  static const char* UKNG[]= {
    "SV","SW","SX","SY","SZ","TV","TW","SQ","SR","SS","ST","SU","TQ","TR",
    "SL","SM","SN","SO","SP","TL","TM","SF","SG","SH","SJ","SK","TF","TG",
    "SA","SB","SC","SD","SE","TA","TB","NV","NW","NX","NY","NZ","OV","OW",
    "NQ","NR","NS","NT","NU","OQ","OR","NL","NM","NN","NO","NP","OL","OM",
    "NF","NG","NH","NJ","NK","OF","OG","NA","NB","NC","ND","NE","OA","OB",
    "HV","HW","HX","HY","HZ","JV","JW","HQ","HR","HS","HT","HU","JQ","JR",
    "HL","HM","HN","HO","HP","JL","JM",""
  };



#endif
