studio.plugins.registerPluginDescription("AudioGaming AudioWind", {
	companyName: "AudioGaming",
	productName: "AudioWind",

	parameters: {
		"GlobalGain":             { displayName: "Gain"         },
		"GlobalSpeed":            { displayName: "Speed"        },
		"AutoToManual":           { displayName: "Auto"         },
		"GlobalAttack":           { displayName: "Attack"       },
		"GlobalRate":             { displayName: "Rate"         },
		"GlobalMin":              { displayName: "Min"          },
		"GlobalMax":              { displayName: "Max"          },

		"BackflowLevel":          { displayName: "Level"        },
		"BackflowRate":           { displayName: "Rate"         },
		"VariationRange":         { displayName: "Var"          },
		"BackflowFreq":           { displayName: "Freq"         },
		"BackflowQ":              { displayName: "Q"            },
		"BackflowLoCut":          { displayName: "LoCut"        },
		"BackflowHiCut":          { displayName: "HiCut"        },
		"BackflowCoupl":          { displayName: "Global"       },
		"BackflowShape":          { displayName: "Shape"        },
		"BackflowFilter":         { displayName: "FilterType"   },

		"WhistlingLevel":         { displayName: "Level"        },
		"WhistlingRate":          { displayName: "Rate"         },
		"WhistlingPres":          { displayName: "Presence"     },
		"WhistlingFloor":         { displayName: "Floor"        },
		"WhistlingQ":             { displayName: "Q"            },
		"WhistlingFmin":          { displayName: "FMin"         },
		"WhistlingFmax":          { displayName: "FMax"         },
		"WhistlingLoCut":         { displayName: "LoCut"        },
		"WhistlingHiCut":         { displayName: "HiCut"        },
		"WhistlingCoupl":         { displayName: "Global"       },
		"WhistlingShape":         { displayName: "Shape"        },

		"GustsLevel":             { displayName: "Level"        },
		"GustsAttack":            { displayName: "Attack"       },
		"GustsRate":              { displayName: "Rate"         },
		"GustsMin":               { displayName: "Min"          },
		"GustsMax":               { displayName: "Max"          },
		"GustsLoCut":             { displayName: "LoCut"        },
		"GustsHiCut":             { displayName: "HiCut"        },
		"GustsCoupl":             { displayName: "Global"       },
		"GustsShape":             { displayName: "Shape"        },

	        "SquallsLevel":           { displayName: "Level"        },
		"SquallsAmp":             { displayName: "Amp"          },
		"SquallsFloor":           { displayName: "Floor"        },
		"SquallsRate":            { displayName: "Rate"         },
		"SquallsLoCut":           { displayName: "LoCut"        },
		"SquallsHiCut":           { displayName: "HiCut"        },
		"SquallsCoupl":           { displayName: "Global"       },
		"SquallsShape":           { displayName: "Shape"        },

		"LFE":                    { displayName: "Level"        },
	        "LFECut":                 { displayName: "LoCut"        },
	        "Format":                 { displayName: "Channels"     },
	},

	deckUi: 
	{
        // \\BOITE HORIZONTALE QUI CONTIENT L'INTERFACE DE TOUT LE PLUGIN
        deckWidgetType: studio.ui.deckWidgetType.Layout,
        layout: studio.ui.layoutType.HBoxLayout,
        spacing: 20,
        items: [
        {
        	// \\BOITE VERTICALE QUI CONTIENT LE LOGO
        	deckWidgetType: studio.ui.deckWidgetType.Layout,
        	layout: studio.ui.layoutType.GridLayout,
        	contentsMargins: {left: 13},
        	spacing: 20,   
        	items: [
        	{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/audiogaming_audiowind_logo.png' },
        	{ deckWidgetType: studio.ui.deckWidgetType.Dropdown, binding: "Format", minimumWidth: 52,                               },
        	],
        },

        {
            // \\BOITE HORIZONTALE QUI CONTIENT LES CONTROLES GLOBAUX
            deckWidgetType: studio.ui.deckWidgetType.Layout,
            layout: studio.ui.layoutType.HBoxLayout, 
            items: [
            {
            	// \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	deckWidgetType: studio.ui.deckWidgetType.Layout,
            	layout: studio.ui.layoutType.VBoxLayout,
            	items: [
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout, 
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	{
            	   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	   deckWidgetType: studio.ui.deckWidgetType.Layout,
            	   layout: studio.ui.layoutType.HBoxLayout, 
            	   items: [
            	   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/global.png'                 },
            	   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "GlobalGain"                                },
            	   ],
            	},
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	],
            },
            {
                // \\BOITE VERTICALE QUI CONTIENT DEUX BOITES HORIZONTALE: DEUX LIGNES DE KNOBS
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
		spacing: 10,
                items: [
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA PREMIERE LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
					spacing: 6,
                    items: [
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF3300", binding: "GlobalSpeed"          },
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF3300", binding: "GlobalMin"            },
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF3300", binding: "GlobalMax"            },
                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
					spacing: 6,
                    items: [
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF3300", binding: "AutoToManual" },
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF3300", binding: "GlobalAttack"       },
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF3300", binding: "GlobalRate"         },
                    ],
                }
                ],
            },
            ],
        },


        {
            // \\BOITE HORIZONTALE QUI CONTIENT LES CONTROLES DE BACKFLOW
            deckWidgetType: studio.ui.deckWidgetType.Layout,
            layout: studio.ui.layoutType.HBoxLayout,
            items: [
            {
            	// \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	deckWidgetType: studio.ui.deckWidgetType.Layout,
            	layout: studio.ui.layoutType.VBoxLayout,
            	items: [
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout, 
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	{
            	   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	   deckWidgetType: studio.ui.deckWidgetType.Layout,
            	   layout: studio.ui.layoutType.HBoxLayout, 
            	   items: [
            	   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/background.png'                 },
            	   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "BackflowLevel"                                },
            	   ],
            	},
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	],
            },
            {
                // \\BOITE VERTICALE QUI CONTIENT DEUX BOITES HORIZONTALE: DEUX LIGNES DE KNOBS
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
		spacing: 10,
                items: [
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA PREMIERE LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
					layout: studio.ui.layoutType.HBoxLayout,
					spacing: 3,
                    items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "BackflowCoupl"  },
		    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "BackflowFreq"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "BackflowQ"            },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "BackflowRate"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "VariationRange"       },
                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
					layout: studio.ui.layoutType.HBoxLayout,
					spacing: 3,
                    items: [

                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "BackflowLoCut"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "BackflowHiCut"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "BackflowFilter"    },
  		    { deckWidgetType: studio.ui.deckWidgetType.Dropdown,  minimumWidth: 80, color: "#057AFF", binding: "BackflowShape"         },

                    ],
                },
                ],
            },
            ],
        },


        {

            // \\BOITE HORIZONTALE QUI CONTIENT LES CONTROLES DE WHISTLING
            deckWidgetType: studio.ui.deckWidgetType.Layout,
            layout: studio.ui.layoutType.HBoxLayout,
            items: [
            {
            	// \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	deckWidgetType: studio.ui.deckWidgetType.Layout,
            	layout: studio.ui.layoutType.VBoxLayout,
            	items: [
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout, 
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	{
            	   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	   deckWidgetType: studio.ui.deckWidgetType.Layout,
            	   layout: studio.ui.layoutType.HBoxLayout, 
            	   items: [
            	   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/whistling.png'                 },
            	   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "WhistlingLevel"                                },
            	   ],
            	},
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	],
            },
            {
                // \\BOITE VERTICALE QUI CONTIENT DEUX BOITES HORIZONTALE: DEUX LIGNES DE KNOBS
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
		spacing: 10,
                items: [
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA PREMIERE LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
					layout: studio.ui.layoutType.HBoxLayout,
					spacing: 3,
                    items: [
		    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "WhistlingCoupl" },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "WhistlingQ"           },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "WhistlingRate"        },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "WhistlingFmin"        },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "WhistlingFmax"        },

                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
					layout: studio.ui.layoutType.HBoxLayout,
					spacing: 3,
                    items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "WhistlingPres"      },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "WhistlingFloor"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "WhistlingLoCut"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "WhistlingHiCut"         },
		    { deckWidgetType: studio.ui.deckWidgetType.Dropdown,   minimumWidth: 80, color: "#00ff7f", binding: "WhistlingShape"         },
                    ],
                },
                ],
            },
            ],
        },


        {

            // \\BOITE HORIZONTALE QUI CONTIENT LES CONTROLES DE GUSTS
            deckWidgetType: studio.ui.deckWidgetType.Layout,
            layout: studio.ui.layoutType.HBoxLayout,
            items: [
            {
            	// \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	deckWidgetType: studio.ui.deckWidgetType.Layout,
            	layout: studio.ui.layoutType.VBoxLayout,
            	items: [
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout, 
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	{
            	   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	   deckWidgetType: studio.ui.deckWidgetType.Layout,
            	   layout: studio.ui.layoutType.HBoxLayout, 
            	   items: [
            	   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/gusts.png'                 },
            	   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "GustsLevel"                                },
            	   ],
            	},
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	],
            },
            {
                // \\BOITE VERTICALE QUI CONTIENT DEUX BOITES HORIZONTALE: DEUX LIGNES DE KNOBS
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
		spacing: 10,
                items: [
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA PREMIERE LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
					layout: studio.ui.layoutType.HBoxLayout,
					spacing: 3,
                    items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#5AC8FA", binding: "GustsCoupl" },
   		    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#5AC8FA", binding: "GustsMin"             },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#5AC8FA", binding: "GustsMax"             },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#5AC8FA", binding: "GustsRate"            },

                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
					layout: studio.ui.layoutType.HBoxLayout,
					spacing: 3,
                    items: [
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#5AC8FA", binding: "GustsAttack"          },
			{ deckWidgetType: studio.ui.deckWidgetType.Dial,color: "#5AC8FA", binding: "GustsLoCut"         },
   			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#5AC8FA", binding: "GustsHiCut"         },
			{ deckWidgetType: studio.ui.deckWidgetType.Dropdown,  minimumWidth: 90, color: "#5AC8FA", binding: "GustsShape"         },
                    ],
                },
                ],
            },
            ],
        },

        {

            // \\BOITE HORIZONTALE QUI CONTIENT LES CONTROLES DE SQUALLS
            deckWidgetType: studio.ui.deckWidgetType.Layout,
            layout: studio.ui.layoutType.HBoxLayout,
            items: [
            {
            	// \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	deckWidgetType: studio.ui.deckWidgetType.Layout,
            	layout: studio.ui.layoutType.VBoxLayout,
            	items: [
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout, 
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	{
            	   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	   deckWidgetType: studio.ui.deckWidgetType.Layout,
            	   layout: studio.ui.layoutType.HBoxLayout, 
            	   items: [
            	   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/squalls.png'                 },
            	   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "SquallsLevel"                                },
            	   ],
            	},
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'                 },
            		],
            	},
            	],
            },
            {
                // \\BOITE VERTICALE QUI CONTIENT DEUX BOITES HORIZONTALE: DEUX LIGNES DE KNOBS
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
		spacing: 10,
                items: [
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA PREMIERE LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
		    spacing: 3,
                    items: [
		    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF9500", binding: "SquallsCoupl" },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF9500", binding: "SquallsAmp"           },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF9500", binding: "SquallsRate"          },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF9500", binding: "SquallsFloor"         },
                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
					layout: studio.ui.layoutType.HBoxLayout,
					spacing: 3,
                    items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                       color: "#FF9500", binding: "SquallsLoCut"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                       color: "#FF9500", binding: "SquallsHiCut"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dropdown,  minimumWidth: 80, color: "#FF9500", binding: "SquallsShape"         },
                    ],
                },
                ],
            },
            ],
        },


        {

        	// \\BOITE HORIZONTALE QUI CONTIENT LE LABEL
        	deckWidgetType: studio.ui.deckWidgetType.Layout,
        	layout: studio.ui.layoutType.HBoxLayout,
        	spacing: 5,
        	contentsMargins: {left: 2},
        	items: [
        	{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/LFE.png'      },
        	{
        	    // \\BOITE VERTICALE QUI CONTIENT LES CONTROLES DU LFE
        	    deckWidgetType: studio.ui.deckWidgetType.Layout,
        	    layout: studio.ui.layoutType.VBoxLayout,
		    spacing: 10,
        	    items: [
        	    {
			// \\BOITE HORIZONTALE QUI CONTIENT LE FADER DE VOLUME
			deckWidgetType: studio.ui.deckWidgetType.Layout,
			layout: studio.ui.layoutType.HBoxLayout,
			items: [
			    { deckWidgetType: studio.ui.deckWidgetType.Dial,                  binding: "LFE"    },
			],
		    },
		    {
            		// \\BOITE HORIZONTALE QUI CONTIENT LE KNOB DE LOW CUT
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FFFFFF", binding: "LFECut" },
            		],
		    },
            ],
        },
        ],

    }

    ],
},



defaultConfigurations: [

{
	name:"Blizzard",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 100,
		"AutoToManual": 80, // Most Manual
		"GlobalAttack": 83,
		"GlobalRate": 50,
		"GlobalMin": 10,
		"GlobalMax": 100,

		"BackflowLevel": -14,
		"BackflowRate": 31,
		"VariationRange": 55,
		"BackflowFreq": 51,
		"BackflowQ": 24,
		"BackflowLoCut": 20000,
		"BackflowHiCut": 461,
		"BackflowCoupl": 0, // Local
		"BackflowShape": 10, // Blizzard
		"BackflowFilter": 0, // Low Pass

		"WhistlingLevel": -7,
		"WhistlingRate": 50,
		"WhistlingPres": 57,
		"WhistlingFloor": 19,
		"WhistlingQ": 15,
		"WhistlingFmin": 325,
		"WhistlingFmax": 2531,
		"WhistlingLoCut": 50,
		"WhistlingHiCut": 20000,
		"WhistlingCoupl": 90, // Global
		"WhistlingShape": 1, // Large

		"GustsLevel": -100,
		"GustsAttack": 51,
		"GustsRate": 27,
		"GustsMin": 21,
		"GustsMax": 63,
		"GustsLoCut": 50,
		"GustsHiCut": 20000,
		"GustsCoupl": 0, // Local
		"GustsShape": 10, // Blizzard

		"SquallsLevel": -5,
		"SquallsAmp": 41,
		"SquallsFloor": 5,
		"SquallsRate": 8,
		"SquallsLoCut": 50,
		"SquallsHiCut": 298,
		"SquallsCoupl": 0, // Local
		"SquallsShape": 2, // Narrow

		"LFE": -100,
		"LFECut": 120,
	},
},

{
	name:"Snow Storm",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 100,
		"AutoToManual": 30,
		"GlobalAttack": 62,
		"GlobalRate": 53,
		"GlobalMin": 23,
		"GlobalMax": 100,

		"BackflowLevel": 0,
		"BackflowRate": 53,
		"VariationRange": 93,
		"BackflowFreq": 100,
		"BackflowQ": 0,
		"BackflowLoCut": 50,
		"BackflowHiCut": 14234,
		"BackflowCoupl": 0,
		"BackflowShape": 0, // Flatlands Bass
		"BackflowFilter": 0, // Low Pass

		"WhistlingLevel": 0,
		"WhistlingRate": 72,
		"WhistlingPres": 82,
		"WhistlingFloor": 78,
		"WhistlingQ": 8,
		"WhistlingFmin": 368,
		"WhistlingFmax": 3080,
		"WhistlingLoCut": 50,
		"WhistlingHiCut": 20000,
		"WhistlingCoupl": 35,
		"WhistlingShape": 1, // Large

		"GustsLevel": 0,
		"GustsAttack": 83,
		"GustsRate": 50,
		"GustsMin": 32,
		"GustsMax": 100,
		"GustsLoCut": 50,
		"GustsHiCut": 20000,
		"GustsCoupl": 57,
		"GustsShape": 6, // Arctic

		"SquallsLevel": 0,
		"SquallsAmp": 100,
		"SquallsFloor": 58,
		"SquallsRate": 23,
		"SquallsLoCut": 50,
		"SquallsHiCut": 20000,
		"SquallsCoupl": 0,
		"SquallsShape": 2, // Narrow

		"LFE": -100,
		"LFECut": 120,
	},
},

{
	name:"Sand Storm",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 100,
		"AutoToManual": 0,
		"GlobalAttack": 58,
		"GlobalRate": 61,
		"GlobalMin": 50,
		"GlobalMax": 100,

		"BackflowLevel": -5,
		"BackflowRate": 50,
		"VariationRange": 50,
		"BackflowFreq": 43,
		"BackflowQ": 100,
		"BackflowLoCut": 50,
		"BackflowHiCut": 274,
		"BackflowCoupl": 0,
		"BackflowShape": 25, // Desert Low
		"BackflowFilter": 2, // All-Pass

		"WhistlingLevel": -5,
		"WhistlingRate": 63,
		"WhistlingPres": 100,
		"WhistlingFloor": 21,
		"WhistlingQ": 1,
		"WhistlingFmin": 361,
		"WhistlingFmax": 5000,
		"WhistlingLoCut": 69,
		"WhistlingHiCut": 20000,
		"WhistlingCoupl": 30,
		"WhistlingShape": 0, // Medium

		"GustsLevel": 0,
		"GustsAttack": 88,
		"GustsRate": 76,
		"GustsMin": 54,
		"GustsMax": 100,
		"GustsLoCut": 50,
		"GustsHiCut": 2672,
		"GustsCoupl": 50,
		"GustsShape": 25, // Desert Low

		"SquallsLevel": 0,
		"SquallsAmp": 100,
		"SquallsFloor": 100,
		"SquallsRate": 36,
		"SquallsLoCut": 50,
		"SquallsHiCut": 111,
		"SquallsCoupl": 50,
		"SquallsShape": 1, // Large

		"LFE": -12,
		"LFECut": 32,
	},
},

{
	name:"Forest",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 100,
		"AutoToManual": 20,
		"GlobalAttack": 83,
		"GlobalRate": 55,
		"GlobalMin": 24,
		"GlobalMax": 67,

		"BackflowLevel": -22,
		"BackflowRate": 43,
		"VariationRange": 72,
		"BackflowFreq": 50,
		"BackflowQ": 13,
		"BackflowLoCut": 150,
		"BackflowHiCut": 7052,
		"BackflowCoupl": 0,
		"BackflowShape": 3, // Forest Bass
		"BackflowFilter": 2, // All-Pass

		"WhistlingLevel": -8,
		"WhistlingRate": 50,
		"WhistlingPres": 69,
		"WhistlingFloor": 0,
		"WhistlingQ": 50,
		"WhistlingFmin": 221,
		"WhistlingFmax": 1231,
		"WhistlingLoCut": 309,
		"WhistlingHiCut": 14115,
		"WhistlingCoupl": 90,
		"WhistlingShape": 1, // Large

		"GustsLevel": -8,
		"GustsAttack": 83,
		"GustsRate": 50,
		"GustsMin": 0,
		"GustsMax": 100,
		"GustsLoCut": 210,
		"GustsHiCut": 14056,
		"GustsCoupl": 90,
		"GustsShape": 5, // Forest

		"SquallsLevel": -8,
		"SquallsAmp": 100,
		"SquallsFloor": 0,
		"SquallsRate": 48,
		"SquallsLoCut": 90,
		"SquallsHiCut": 20000,
		"SquallsCoupl": 100, 
		"SquallsShape": 0, // Medium

		"LFE": -100,
		"LFECut": 120,
	},
},

{
	name:"Mountain",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 100,
		"AutoToManual": 0,
		"GlobalAttack": 32,
		"GlobalRate": 50,
		"GlobalMin": 0,
		"GlobalMax": 100,

		"BackflowLevel": -11,
		"BackflowRate": 50,
		"VariationRange": 50,
		"BackflowFreq": 73,
		"BackflowQ": 20,
		"BackflowLoCut": 50,
		"BackflowHiCut": 20000,
		"BackflowCoupl": 20,
		"BackflowShape": 0, // Flatlands Bass
		"BackflowFilter": 0, // Low-Pass

		"WhistlingLevel": -2,
		"WhistlingRate": 92,
		"WhistlingPres": 80,
		"WhistlingFloor": 40,
		"WhistlingQ": 48,
		"WhistlingFmin": 313,
		"WhistlingFmax": 2791,
		"WhistlingLoCut": 50,
		"WhistlingHiCut": 14075,
		"WhistlingCoupl": 0,
		"WhistlingShape": 1, // Large

		"GustsLevel": -9,
		"GustsAttack": 86,
		"GustsRate": 100,
		"GustsMin": 100,
		"GustsMax": 100,
		"GustsLoCut": 50,
		"GustsHiCut": 14075,
		"GustsCoupl": 70,
		"GustsShape": 7, // Steppes

		"SquallsLevel": -3,
		"SquallsAmp": 66,
		"SquallsFloor": 27,
		"SquallsRate": 100,
		"SquallsLoCut": 50,
		"SquallsHiCut": 6155,
		"SquallsCoupl": 0,
		"SquallsShape": 0, // Medium

		"LFE": -8,
		"LFECut": 212,
	},
},

{
	name:"Seaside",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 0.01,
		"AutoToManual": 0,
		"GlobalAttack": 88,
		"GlobalRate": 59,
		"GlobalMin": 48,
		"GlobalMax": 28,

		"BackflowLevel": -5,
		"BackflowRate": 74,
		"VariationRange": 46,
		"BackflowFreq": 74,
		"BackflowQ": 60,
		"BackflowLoCut": 50,
		"BackflowHiCut": 13809,
		"BackflowCoupl": 85,
		"BackflowShape": 12, // Sea Low
		"BackflowFilter": 2, // All-Pass

		"WhistlingLevel": 0,
		"WhistlingRate": 74,
		"WhistlingPres": 57,
		"WhistlingFloor": 15,
		"WhistlingQ": 43,
		"WhistlingFmin": 318,
		"WhistlingFmax": 2430,
		"WhistlingLoCut": 50,
		"WhistlingHiCut": 13809,
		"WhistlingCoupl": 20,
		"WhistlingShape": 2, // Narrow

		"GustsLevel": 0,
		"GustsAttack": 72,
		"GustsRate": 49,
		"GustsMin": 15,
		"GustsMax": 17,
		"GustsLoCut": 50,
		"GustsHiCut": 13809,
		"GustsCoupl": 70,
		"GustsShape": 12, // Sea Low

		"SquallsLevel": 0,
		"SquallsAmp": 14,
		"SquallsFloor": 69,
		"SquallsRate": 43,
		"SquallsLoCut": 1173,
		"SquallsHiCut": 11287,
		"SquallsCoupl": 80,
		"SquallsShape": 2, // Narrow

		"LFE": -5,
		"LFECut": 120,
	},
},

{
	name:"Castaway Beach",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 44,
		"AutoToManual": 0,
		"GlobalAttack": 35,
		"GlobalRate": 11,
		"GlobalMin": 24,
		"GlobalMax": 54,

		"BackflowLevel": -4,
		"BackflowRate": 82,
		"VariationRange": 43,
		"BackflowFreq": 22,
		"BackflowQ": 60,
		"BackflowLoCut": 252,
		"BackflowHiCut": 7629,
		"BackflowCoupl": 100,
		"BackflowShape": 4, // Forest HCut
		"BackflowFilter": 1, // Low-Pass

		"WhistlingLevel": 0,
		"WhistlingRate": 65,
		"WhistlingPres": 57,
		"WhistlingFloor": 48,
		"WhistlingQ": 20,
		"WhistlingFmin": 234,
		"WhistlingFmax": 2907,
		"WhistlingLoCut": 5145,
		"WhistlingHiCut": 2938,
		"WhistlingCoupl": 65,
		"WhistlingShape": 0, // Medium

		"GustsLevel": -1,
		"GustsAttack": 56,
		"GustsRate": 59,
		"GustsMin": 29,
		"GustsMax": 85,
		"GustsLoCut": 1192,
		"GustsHiCut": 5069,
		"GustsCoupl": 0,
		"GustsShape": 25, // Desert Low

		"SquallsLevel": -7,
		"SquallsAmp": 81,
		"SquallsFloor": 31,
		"SquallsRate": 47,
		"SquallsLoCut": 50,
		"SquallsHiCut": 664,
		"SquallsCoupl": 0,
		"SquallsShape": 1, // Large

		"LFE": -30,
		"LFECut": 116,
	},
},

{
	name:"Pressure Cooker",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 100,
		"AutoToManual": 100,
		"GlobalAttack": 83,
		"GlobalRate": 50,
		"GlobalMin": 10,
		"GlobalMax": 100,

		"BackflowLevel": 0,
		"BackflowRate": 99,
		"VariationRange": 30,
		"BackflowFreq": 98,
		"BackflowQ": 100,
		"BackflowLoCut": 3357,
		"BackflowHiCut": 16983,
		"BackflowCoupl": 0,
		"BackflowShape": 12, // Sea Low
		"BackflowFilter": 1, // Band Pass Q

		"WhistlingLevel": -100,
		"WhistlingRate": 33,
		"WhistlingPres": 100,
		"WhistlingFloor": 38,
		"WhistlingQ": 100,
		"WhistlingFmin": 470,
		"WhistlingFmax": 2107,
		"WhistlingLoCut": 2083,
		"WhistlingHiCut": 1638,
		"WhistlingCoupl": 0,
		"WhistlingShape": 2, // Narrow

		"GustsLevel": -100,
		"GustsAttack": 95,
		"GustsRate": 16,
		"GustsMin": 59,
		"GustsMax": 100,
		"GustsLoCut": 444,
		"GustsHiCut": 203,
		"GustsCoupl": 0,
		"GustsShape": 0, // Flatlands Bass

		"SquallsLevel": -100,
		"SquallsAmp": 100,
		"SquallsFloor": 60,
		"SquallsRate": 22,
		"SquallsLoCut": 3078,
		"SquallsHiCut": 20000,
		"SquallsCoupl": 0,
		"SquallsShape": 0, // Medium

		"LFE": -100,
		"LFECut": 20,
	},
},

{
	name:"Jupiter Landing",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 31,
		"AutoToManual": 10,
		"GlobalAttack": 47,
		"GlobalRate": 57,
		"GlobalMin": 15,
		"GlobalMax": 19,

		"BackflowLevel": -4,
		"BackflowRate": 38,
		"VariationRange": 68,
		"BackflowFreq": 67,
		"BackflowQ": 44,
		"BackflowLoCut": 6974,
		"BackflowHiCut": 1122,
		"BackflowCoupl": 0,
		"BackflowShape": 20, // Noisy High
		"BackflowFilter": 1, // Band Pass Q

		"WhistlingLevel": -100,
		"WhistlingRate": 2,
		"WhistlingPres": 65,
		"WhistlingFloor": 69,
		"WhistlingQ": 17,
		"WhistlingFmin": 207,
		"WhistlingFmax": 2418,
		"WhistlingLoCut": 1977,
		"WhistlingHiCut": 20000,
		"WhistlingCoupl": 20,
		"WhistlingShape": 0, // Medium

		"GustsLevel": 0,
		"GustsAttack": 28,
		"GustsRate": 62,
		"GustsMin": 31,
		"GustsMax": 78,
		"GustsLoCut": 4580,
		"GustsHiCut": 641,
		"GustsCoupl": 0,
		"GustsShape": 8, // Steppes Far

		"SquallsLevel": -3,
		"SquallsAmp": 100,
		"SquallsFloor": 62,
		"SquallsRate": 5,
		"SquallsLoCut": 50,
		"SquallsHiCut": 20000,
		"SquallsCoupl": 0,
		"SquallsShape": 0, // Medium

		"LFE": -30,
		"LFECut": 116,
	},
},

{
	name:"Weird Twister",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalSpeed": 5,
		"AutoToManual": 0,
		"GlobalAttack": 46,
		"GlobalRate": 100,
		"GlobalMin": 0,
		"GlobalMax": 100,

		"BackflowLevel": 0,
		"BackflowRate": 59,
		"VariationRange": 51,
		"BackflowFreq": 0,
		"BackflowQ": 63,
		"BackflowLoCut": 50,
		"BackflowHiCut": 5718,
		"BackflowCoupl": 70,
		"BackflowShape": 0, // Flatlands Bass
		"BackflowFilter": 1, // Band Pass Q

		"WhistlingLevel": 0,
		"WhistlingRate": 100,
		"WhistlingPres": 100,
		"WhistlingFloor": 100,
		"WhistlingQ": 40,
		"WhistlingFmin": 200,
		"WhistlingFmax": 4315,
		"WhistlingLoCut": 50,
		"WhistlingHiCut": 20000,
		"WhistlingCoupl": 55,
		"WhistlingShape": 1, // Large

		"GustsLevel": 0,
		"GustsAttack": 53,
		"GustsRate": 100,
		"GustsMin": 53,
		"GustsMax": 100,
		"GustsLoCut": 50,
		"GustsHiCut": 20000,
		"GustsCoupl": 55,
		"GustsShape": 20, // Noisy High

		"SquallsLevel": 0,
		"SquallsAmp": 100,
		"SquallsFloor": 37,
		"SquallsRate": 100,
		"SquallsLoCut": 16704,
		"SquallsHiCut": 50,
		"SquallsCoupl": 55,
		"SquallsShape": 1, // Large

		"LFE": 0,
		"LFECut": 120,
	},
},

],
});
