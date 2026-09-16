studio.plugins.registerPluginDescription("AudioGaming AudioRain", {
	companyName: "AudioGaming",
	productName: "AudioRain",


	parameters: {
		"GlobalGain":             { displayName: "Level"        },
	        "GlobalStrength":         { displayName: "Strength"     },
		"AutoToManual":           { displayName: "Auto"         },
		"GlobalAttack":           { displayName: "Attack"       },
		"GlobalRate":             { displayName: "Rate"         },
		"GlobalMin":              { displayName: "Min"          },
		"GlobalMax":              { displayName: "Max"          },

		"RumbleLevel":            { displayName: "Level"        },
		"RumbleDensity":          { displayName: "Densty"       },
		"RumbleNotchQ":           { displayName: "NotchQ"       },
		"RumbleHiCut":            { displayName: "HiCut"        },
		"RumbleLoCut":            { displayName: "LoCut"        },
		"RumbleCoupl":            { displayName: "Global"  }, 

		"ShowerLevel":            { displayName: "Level"        },
		"ShowerColor":            { displayName: "Color"        },
		"ShowerHiCut":            { displayName: "HiCut"        },
		"ShowerLoCut":            { displayName: "LoCut"        },

		"Liquid1Level":           { displayName: "Level"        },
		"Liquid1Density":         { displayName: "Density"      },
		"Liquid1Color":           { displayName: "Color"        },
		"Liquid1Visc":            { displayName: "Visc."        },
		"Liquid1Min":             { displayName: "Min"          },
		"Liquid1Max":             { displayName: "Max"          },
		"Liquid1HiCut":           { displayName: "HiCut"        },
		"Liquid1LoCut":           { displayName: "LoCut"        },

		"Liquid2Level":           { displayName: "Level"        },
		"Liquid2Density":         { displayName: "Density"      },
		"Liquid2Color":           { displayName: "Color"        },
		"Liquid2Visc":            { displayName: "Visc."        },
		"Liquid2Min":             { displayName: "Min"          },
		"Liquid2Max":             { displayName: "Max"          },
		"Liquid2HiCut":           { displayName: "HiCut"        },
		"Liquid2LoCut":           { displayName: "LoCut"        },

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
        	layout: studio.ui.layoutType.VBoxLayout,
        	contentsMargins: {left: 13},
        	spacing: "20",
        	items: [
        	{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/audiogaming_audiorain_logo.png'  },
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
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'          },
            		],
            	},
            	{
            	   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	   deckWidgetType: studio.ui.deckWidgetType.Layout,
            	   layout: studio.ui.layoutType.HBoxLayout, 
            	   items: [
            	   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/global.png'               },
            	   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "GlobalGain"            },
            	   ],
            	},
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'          },
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
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF3300", binding: "GlobalStrength"           },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF3300", binding: "GlobalMin"             },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF3300", binding: "GlobalMax"             },
                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
		    spacing: 6,
                    items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,   color: "#FF3300", binding: "AutoToManual"  },
		    { deckWidgetType: studio.ui.deckWidgetType.Dial,   color: "#FF3300", binding: "GlobalAttack"        },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,   color: "#FF3300", binding: "GlobalRate"          },
                    ],
                }
                ],
            },
            ],
        },

        {
            // \\BOITE HORIZONTALE QUI CONTIENT LES CONTROLES DE RUMBLE
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
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'          },
            		],
            	},
            	{
            	   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	   deckWidgetType: studio.ui.deckWidgetType.Layout,
            	   layout: studio.ui.layoutType.HBoxLayout, 
            	   items: [
            	   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/rumble.png'           },
            	   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "RumbleLevel"           },
            	   ],
            	},
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'          },
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
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "RumbleCoupl" },
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "RumbleDensity"         },
			{ deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#057AFF", binding: "RumbleNotchQ"          },
                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
		    spacing: 6,
                    items: [
			{ deckWidgetType: studio.ui.deckWidgetType.Dial,   color: "#057AFF", binding: "RumbleLoCut"         },
			{ deckWidgetType: studio.ui.deckWidgetType.Dial,   color: "#057AFF", binding: "RumbleHiCut"         },
                    ],
                },
                ],
            },
            ],
        },


        {

            // \\BOITE HORIZONTALE QUI CONTIENT LES CONTROLES DE SHOWER
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
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'         },
            		],
            	},
            	{
            	   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	   deckWidgetType: studio.ui.deckWidgetType.Layout,
            	   layout: studio.ui.layoutType.HBoxLayout, 
            	   items: [
            	   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/shower.png'           },
            	   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "ShowerLevel"          },
            	   ],
            	},
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'         },
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
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#00ff7f", binding: "ShowerColor"          },
                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
		    spacing: 6,
                    items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                           color: "#00ff7f", binding: "ShowerLoCut" },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                           color: "#00ff7f", binding: "ShowerHiCut" },
                    ],
                },
                ],
            },
            ],
        },


        {

            // \\BOITE HORIZONTALE QUI CONTIENT LES CONTROLES DE LIQUID 1
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
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'         },
            		],
            	},
            	{
            	   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
            	   deckWidgetType: studio.ui.deckWidgetType.Layout,
            	   layout: studio.ui.layoutType.HBoxLayout, 
            	   items: [
            	   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/liquid1.png'               },
            	   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "Liquid1Level"         },
            	   ],
            	},
            	{
            		// \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
            		deckWidgetType: studio.ui.deckWidgetType.Layout,
            		layout: studio.ui.layoutType.HBoxLayout,
            		items: [
            		{ deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'         },
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
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#5AC8FA", binding: "Liquid1Density"       },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#5AC8FA", binding: "Liquid1Color"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#5AC8FA", binding: "Liquid1Visc"          },
                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
		    spacing: 6,
                    items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                       color: "#5AC8FA", binding: "Liquid1Min"      },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                       color: "#5AC8FA", binding: "Liquid1Max"      },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                     color: "#5AC8FA", binding: "Liquid1LoCut"    },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, stretchFactor: 1, color: "#5AC8FA", binding: "Liquid1HiCut"    },
                    ],
                },
                ],
            },
            ],
        },

        {

            // \\BOITE HORIZONTALE QUI CONTIENT LES CONTROLES DE LIQUID 1
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
                    { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'         },
                    ],
                },
                {
                   // \\BOITE VERTICALE QUI CONTIENT LE FADER DE VOLUME ET LE LABEL
                   deckWidgetType: studio.ui.deckWidgetType.Layout,
                   layout: studio.ui.layoutType.HBoxLayout, 
                   items: [
                   { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/liquid2.png'               },
                   { deckWidgetType: studio.ui.deckWidgetType.Fader, stretchFactor: 1, binding: "Liquid2Level"         },
                   ],
               },
               {
                    // \\BOITE VERTICALE QUI CONTIENT UNE IMAGE TEST
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
                    items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + '/audiogaming/fader_lock.png'         },
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
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF9500", binding: "Liquid2Density"       },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF9500", binding: "Liquid2Color"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#FF9500", binding: "Liquid2Visc"          },
                    ],
                },
                {
                    // \\BOITE HORIZONTALE QUI CONTIENT LA DEUXIEME LIGNE DE KNOBS
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
		    spacing: 6,
                    items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                       color: "#FF9500", binding: "Liquid2Min"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                       color: "#FF9500", binding: "Liquid2Max"         },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial,                     color: "#FF9500", binding: "Liquid2LoCut"       },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, stretchFactor: 1, color: "#FF9500", binding: "Liquid2HiCut"       },
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
	name:"Jungle",
	parameterValues: {

		"GlobalGain": 0,
		"GlobalStrength": 72,
		"AutoToManual": 0,
		"GlobalAttack": 59,
		"GlobalRate": 65,
		"GlobalMin": 52,
		"GlobalMax": 100,

		"RumbleLevel": 0,
		"RumbleDensity": 28,
		"RumbleNotchQ": 49,
		"RumbleHiCut": 7362,
		"RumbleLoCut": 132,
		"RumbleCoupl": 0,

		"ShowerLevel": -3.22,
		"ShowerColor": 300,
		"ShowerHiCut": 3534,
		"ShowerLoCut": 102,

		"Liquid1Level": 0,
		"Liquid1Density": 83,
		"Liquid1Color": 803,
		"Liquid1Visc": 263,
		"Liquid1Min": 0,
		"Liquid1Max": 56,
		"Liquid1HiCut": 4215,
		"Liquid1LoCut": 250, 

		"Liquid2Level": -1.98,
		"Liquid2Density": 100,
		"Liquid2Color": 964,
		"Liquid2Visc": 289,
		"Liquid2Min": 40,
		"Liquid2Max": 100,
		"Liquid2HiCut": 4155,
		"Liquid2LoCut": 250, 

		"LFE": -100,
		"LFECut": 49,
	},
},


{
	name:"City",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 24,
		"AutoToManual": 100,
		"GlobalAttack": 55,
		"GlobalRate": 57,
		"GlobalMin": 0,
		"GlobalMax": 18,

		"RumbleLevel": -7.33,
		"RumbleDensity": 30,
		"RumbleNotchQ": 15,
		"RumbleHiCut": 8057,
		"RumbleLoCut": 221,
		"RumbleCoupl": 100,

		"ShowerLevel": 0,
		"ShowerColor": 362,
		"ShowerHiCut": 988,
		"ShowerLoCut": 1229,

		"Liquid1Level": -8.56,
		"Liquid1Density": 71,
		"Liquid1Color": 179,
		"Liquid1Visc": 177,
		"Liquid1Min": 62,
		"Liquid1Max": 100,
		"Liquid1HiCut": 4049,
		"Liquid1LoCut": 395, 

		"Liquid2Level": -100,
		"Liquid2Density": 0,
		"Liquid2Color": 500,
		"Liquid2Visc": 1,
		"Liquid2Min": 100,
		"Liquid2Max": 100,
		"Liquid2HiCut": 4500,
		"Liquid2LoCut": 380, 

		"LFE": -100,
		"LFECut": 120,
	},
},

{
	name:"Lake",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 100,
		"AutoToManual": 0,
		"GlobalAttack": 65,
		"GlobalRate": 71,
		"GlobalMin": 42,
		"GlobalMax": 80,

		"RumbleLevel": -5.55,
		"RumbleDensity": 46,
		"RumbleNotchQ": 0.8,
		"RumbleHiCut": 12116,
		"RumbleLoCut": 443,
		"RumbleCoupl": 0,

		"ShowerLevel": -9.79,
		"ShowerColor": 150,
		"ShowerHiCut": 3775,
		"ShowerLoCut": 1122,

		"Liquid1Level": -6.52,
		"Liquid1Density": 79,
		"Liquid1Color": 374,
		"Liquid1Visc": 20.1,
		"Liquid1Min": 62,
		"Liquid1Max": 95,
		"Liquid1HiCut": 4760,
		"Liquid1LoCut": 1600, 

		"Liquid2Level": -3.86,
		"Liquid2Density": 79,
		"Liquid2Color": 374,
		"Liquid2Visc": 19.7,
		"Liquid2Min": 0,
		"Liquid2Max": 100,
		"Liquid2HiCut": 6000,
		"Liquid2LoCut": 1380, 

		"LFE": -100,
		"LFECut": 49,
	},
},

{
	name:"Mountain",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 100,
		"AutoToManual": 0,
		"GlobalAttack": 59,
		"GlobalRate": 71,
		"GlobalMin": 36,
		"GlobalMax": 68,

		"RumbleLevel": -0.9,
		"RumbleDensity": 55,
		"RumbleNotchQ": 0,
		"RumbleHiCut": 8969,
		"RumbleLoCut": 205,
		"RumbleCoupl": 60,

		"ShowerLevel": -6.02,
		"ShowerColor": 300,
		"ShowerHiCut": 2890,
		"ShowerLoCut": 762,

		"Liquid1Level": -6.78,
		"Liquid1Density": 99,
		"Liquid1Color": 694,
		"Liquid1Visc": 124,
		"Liquid1Min": 0,
		"Liquid1Max": 52,
		"Liquid1HiCut": 4000,
		"Liquid1LoCut": 913, 

		"Liquid2Level": -100,
		"Liquid2Density": 100,
		"Liquid2Color": 1118,
		"Liquid2Visc": 195,
		"Liquid2Min": 47,
		"Liquid2Max": 100,
		"Liquid2HiCut": 6000,
		"Liquid2LoCut": 1017, 

		"LFE": -8.24,
		"LFECut": 49,
	},
},

{
	name:"Car",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 95,
		"AutoToManual": 95,
		"GlobalAttack": 34,
		"GlobalRate": 100,
		"GlobalMin": 31,
		"GlobalMax": 81,

		"RumbleLevel": -6.14,
		"RumbleDensity": 57,
		"RumbleNotchQ": 48,
		"RumbleHiCut": 6000,
		"RumbleLoCut": 81,
		"RumbleCoupl": 100,

		"ShowerLevel": -5.21,
		"ShowerColor": 300,
		"ShowerHiCut": 913,
		"ShowerLoCut": 51,

		"Liquid1Level": -7.60,
		"Liquid1Density": 44,
		"Liquid1Color": 1169,
		"Liquid1Visc": 124,
		"Liquid1Min": 38,
		"Liquid1Max": 38,
		"Liquid1HiCut": 4000,
		"Liquid1LoCut": 250, 

		"Liquid2Level": -6.56,
		"Liquid2Density": 69,
		"Liquid2Color": 462,
		"Liquid2Visc": 114,
		"Liquid2Min": 33,
		"Liquid2Max": 47,
		"Liquid2HiCut": 4000,
		"Liquid2LoCut": 250, 

		"LFE": -4.69,
		"LFECut": 120,
	},
},

{
	name:"Under a tent",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 100,
		"AutoToManual": 0,
		"GlobalAttack": 16,
		"GlobalRate": 29,
		"GlobalMin": 28,
		"GlobalMax": 60,

		"RumbleLevel": -0.74,
		"RumbleDensity": 83,
		"RumbleNotchQ": 13,
		"RumbleHiCut": 6369,
		"RumbleLoCut": 431,
		"RumbleCoupl": 0,

		"ShowerLevel": -8.09,
		"ShowerColor": 404,
		"ShowerHiCut": 815,
		"ShowerLoCut": 194,

		"Liquid1Level": -22.73,
		"Liquid1Density": 81,
		"Liquid1Color": 66,
		"Liquid1Visc": 98,
		"Liquid1Min": 47,
		"Liquid1Max": 56,
		"Liquid1HiCut": 6000,
		"Liquid1LoCut": 1600, 

		"Liquid2Level": -29.63,
		"Liquid2Density": 82,
		"Liquid2Color": 122,
		"Liquid2Visc": 101,
		"Liquid2Min": 18,
		"Liquid2Max": 33,
		"Liquid2HiCut": 4471,
		"Liquid2LoCut": 250, 

		"LFE": -100,
		"LFECut": 400,
	},
},

{
	name:"Under an umbrella",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 100,
		"AutoToManual": 0,
		"GlobalAttack": 66,
		"GlobalRate": 76,
		"GlobalMin": 10,
		"GlobalMax": 100,

		"RumbleLevel": -9.98,
		"RumbleDensity": 67,
		"RumbleNotchQ": 70,
		"RumbleHiCut": 7145,
		"RumbleLoCut": 393,
		"RumbleCoupl": 0,

		"ShowerLevel": -16.19,
		"ShowerColor": 471,
		"ShowerHiCut": 2932,
		"ShowerLoCut": 2248,

		"Liquid1Level": -3.77,
		"Liquid1Density": 84,
		"Liquid1Color": 164,
		"Liquid1Visc": 400,
		"Liquid1Min": 0,
		"Liquid1Max": 39,
		"Liquid1HiCut": 4159,
		"Liquid1LoCut": 250, 

		"Liquid2Level": -5.66,
		"Liquid2Density": 63,
		"Liquid2Color": 339,
		"Liquid2Visc": 182,
		"Liquid2Min": 30,
		"Liquid2Max": 40,
		"Liquid2HiCut": 4433,
		"Liquid2LoCut": 307, 

		"LFE": -100,
		"LFECut": 120,
	},
},

{
	name:"Hear the drops",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 100,
		"AutoToManual": 0,
		"GlobalAttack": 59,
		"GlobalRate": 37,
		"GlobalMin": 28,
		"GlobalMax": 88,

		"RumbleLevel": -6.45,
		"RumbleDensity": 87,
		"RumbleNotchQ": 26,
		"RumbleHiCut": 6000,
		"RumbleLoCut": 50,
		"RumbleCoupl": 0,

		"ShowerLevel": -20.72,
		"ShowerColor": 549,
		"ShowerHiCut": 8470,
		"ShowerLoCut": 479,

		"Liquid1Level": 0,
		"Liquid1Density": 77,
		"Liquid1Color": 12,
		"Liquid1Visc": 30.9,
		"Liquid1Min": 44,
		"Liquid1Max": 61,
		"Liquid1HiCut": 4296,
		"Liquid1LoCut": 250, 

		"Liquid2Level": 0,
		"Liquid2Density": 100,
		"Liquid2Color": 182,
		"Liquid2Visc": 88.4,
		"Liquid2Min": 49,
		"Liquid2Max": 57,
		"Liquid2HiCut": 4000,
		"Liquid2LoCut": 1600, 

		"LFE": -100,
		"LFECut": 400,
	},
},

{
	name:"Wet Leaves",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 0.17,
		"AutoToManual": 0.9,
		"GlobalAttack": 64,
		"GlobalRate": 86,
		"GlobalMin": 3,
		"GlobalMax": 85,

		"RumbleLevel": -0,
		"RumbleDensity": 24,
		"RumbleNotchQ": 71,
		"RumbleHiCut": 11929,
		"RumbleLoCut": 340,
		"RumbleCoupl": 45,

		"ShowerLevel": -0,
		"ShowerColor": 984,
		"ShowerHiCut": 465,
		"ShowerLoCut": 64,

		"Liquid1Level": 0,
		"Liquid1Density": 40,
		"Liquid1Color": 2656,
		"Liquid1Visc": 124,
		"Liquid1Min": 9,
		"Liquid1Max": 36,
		"Liquid1HiCut": 4991,
		"Liquid1LoCut": 1470, 

		"Liquid2Level": -0,
		"Liquid2Density": 87,
		"Liquid2Color": 4054,
		"Liquid2Visc": 275,
		"Liquid2Min": 26,
		"Liquid2Max": 26,
		"Liquid2HiCut": 4366,
		"Liquid2LoCut": 1148, 

		"LFE": -100,
		"LFECut": 49,
	},
},

{
	name:"Danger Flood",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 100,
		"AutoToManual": 0,
		"GlobalAttack": 16,
		"GlobalRate": 29,
		"GlobalMin": 28,
		"GlobalMax": 60,

		"RumbleLevel": -0.74,
		"RumbleDensity": 84,
		"RumbleNotchQ": 76,
		"RumbleHiCut": 5145,
		"RumbleLoCut": 54,
		"RumbleCoupl": 0,

		"ShowerLevel": -1.36,
		"ShowerColor": 466,
		"ShowerHiCut": 6974,
		"ShowerLoCut": 712,

		"Liquid1Level": -10.81,
		"Liquid1Density": 81,
		"Liquid1Color": 66,
		"Liquid1Visc": 98,
		"Liquid1Min": 47,
		"Liquid1Max": 56,
		"Liquid1HiCut": 4296,
		"Liquid1LoCut": 250, 

		"Liquid2Level": 0,
		"Liquid2Density": 87,
		"Liquid2Color": 122,
		"Liquid2Visc": 124,
		"Liquid2Min": 18,
		"Liquid2Max": 33,
		"Liquid2HiCut": 4904,
		"Liquid2LoCut": 250, 

		"LFE": -100,
		"LFECut": 400,
	},
},

{
	name:"Shower",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 100,
		"AutoToManual": 0,
		"GlobalAttack": 53,
		"GlobalRate": 52,
		"GlobalMin": 39,
		"GlobalMax": 49,

		"RumbleLevel": 0,
		"RumbleDensity": 0,
		"RumbleNotchQ": 100,
		"RumbleHiCut": 20000,
		"RumbleLoCut": 50,
		"RumbleCoupl": 100,

		"ShowerLevel": -0,
		"ShowerColor": 1500,
		"ShowerHiCut": 5675,
		"ShowerLoCut": 479,

		"Liquid1Level": 0,
		"Liquid1Density": 92,
		"Liquid1Color": 127,
		"Liquid1Visc": 61.2,
		"Liquid1Min": 47,
		"Liquid1Max": 100,
		"Liquid1HiCut": 4381,
		"Liquid1LoCut": 702, 

		"Liquid2Level": 0,
		"Liquid2Density": 100,
		"Liquid2Color": 261,
		"Liquid2Visc": 400,
		"Liquid2Min": 100,
		"Liquid2Max": 100,
		"Liquid2HiCut": 6000,
		"Liquid2LoCut": 250, 

		"LFE": -12,
		"LFECut": 400,
	},
},

{
	name:"Cottage Drops",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 63,
		"AutoToManual": 0,
		"GlobalAttack": 59,
		"GlobalRate": 91,
		"GlobalMin": 45,
		"GlobalMax": 48,

		"RumbleLevel": -23.61,
		"RumbleDensity": 34,
		"RumbleNotchQ": 5,
		"RumbleHiCut": 221,
		"RumbleLoCut": 188,
		"RumbleCoupl": 0,

		"ShowerLevel": -2.49,
		"ShowerColor": 887,
		"ShowerHiCut": 149,
		"ShowerLoCut": 51,

		"Liquid1Level": -5.66,
		"Liquid1Density": 79,
		"Liquid1Color": 4448,
		"Liquid1Visc": 376,
		"Liquid1Min": 90,
		"Liquid1Max": 100,
		"Liquid1HiCut": 4000,
		"Liquid1LoCut": 881, 

		"Liquid2Level": -5.66,
		"Liquid2Density": 70,
		"Liquid2Color": 1267,
		"Liquid2Visc": 81.9,
		"Liquid2Min": 60,
		"Liquid2Max": 62,
		"Liquid2HiCut": 4000,
		"Liquid2LoCut": 562, 

		"LFE": -100,
		"LFECut": 49,
	},
},

{
	name:"Storm",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 100,
		"AutoToManual": 20,
		"GlobalAttack": 81,
		"GlobalRate": 64,
		"GlobalMin": 11,
		"GlobalMax": 74,

		"RumbleLevel": 0,
		"RumbleDensity": 60,
		"RumbleNotchQ": 100,
		"RumbleHiCut": 19464,
		"RumbleLoCut": 61,
		"RumbleCoupl": 100,

		"ShowerLevel": -11.21,
		"ShowerColor": 600,
		"ShowerHiCut": 8901,
		"ShowerLoCut": 712,

		"Liquid1Level": -6.52,
		"Liquid1Density": 100,
		"Liquid1Color": 186,
		"Liquid1Visc": 260,
		"Liquid1Min": 68,
		"Liquid1Max": 100,
		"Liquid1HiCut": 4500,
		"Liquid1LoCut": 380, 

		"Liquid2Level": -100,
		"Liquid2Density": 0,
		"Liquid2Color": 500,
		"Liquid2Visc": 1,
		"Liquid2Min": 100,
		"Liquid2Max": 100,
		"Liquid2HiCut": 4500,
		"Liquid2LoCut": 380, 

		"LFE": -100,
		"LFECut": 120,
	},
},

{
	name:"Monsoon",
	parameterValues: {
		"GlobalGain": 0,
		"GlobalStrength": 100,
		"AutoToManual": 60,
		"GlobalAttack": 36,
		"GlobalRate": 60,
		"GlobalMin": 80,
		"GlobalMax": 84,

		"RumbleLevel": -0.90,
		"RumbleDensity": 100,
		"RumbleNotchQ": 100,
		"RumbleHiCut": 16747,
		"RumbleLoCut": 50,
		"RumbleCoupl": 0,

		"ShowerLevel": 0,
		"ShowerColor": 1500,
		"ShowerHiCut": 8915,
		"ShowerLoCut": 50,

		"Liquid1Level": 0,
		"Liquid1Density": 100,
		"Liquid1Color": 23,
		"Liquid1Visc": 298,
		"Liquid1Min": 2,
		"Liquid1Max": 0,
		"Liquid1HiCut": 6000,
		"Liquid1LoCut": 250, 

		"Liquid2Level": 0,
		"Liquid2Density": 100,
		"Liquid2Color": 2477,
		"Liquid2Visc": 151,
		"Liquid2Min": 0,
		"Liquid2Max": 83,
		"Liquid2HiCut": 6000,
		"Liquid2LoCut": 250, 

		"LFE": -5,
		"LFECut": 39,
	},
},


],
});
