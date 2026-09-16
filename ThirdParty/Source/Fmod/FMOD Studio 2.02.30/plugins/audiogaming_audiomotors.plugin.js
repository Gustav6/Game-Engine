studio.plugins.registerPluginDescription("AudioGaming AudioMotors", {
    companyName: "AudioGaming",
    productName: "AudioMotors2",
    parameters: {
        "Randomize": { displayName: "Random" }
    },
    deckUi: {
        deckWidgetType: studio.ui.deckWidgetType.Layout,
        layout: studio.ui.layoutType.HBoxLayout,
        spacing: 18,
        items: [
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                spacing: 20,
                contentsMargins: { left: 20, top: 5, right: 0, bottom: 0 },
                items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + "/audiogaming_audiomotors_logo.png" },
                    { deckWidgetType: studio.ui.deckWidgetType.DataDrop, binding: "Car Data", fileNameFilters: "*.agp" },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.GridLayout,
                spacing: 12,
                items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, color: "#DF171A", row: 0, column: 0, binding: "RPM", },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, row: 0, column: 1, binding: "Transpose", },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, row: 0, column: 2, binding: "Drive", },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, row: 1, column: 0, binding: "Randomize", },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, row: 1, column: 1, binding: "Range", },
                    { deckWidgetType: studio.ui.deckWidgetType.Dial, row: 1, column: 2, binding: "Adapt", },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                spacing: 10,
                contentsMargins: { left: 0, top: 2, right: 0, bottom: 6 },
                items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Button, binding: "Sync", },
                    { deckWidgetType: studio.ui.deckWidgetType.ButtonGroup, binding: "Mode", buttonWidth: 44, },
                    { deckWidgetType: studio.ui.deckWidgetType.Dropdown, binding: "Format", minimumWidth: 52, },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                contentsMargins: { left: 0, top: 8, right: 10, bottom: 10 },
                items: [
                    { deckWidgetType: studio.ui.deckWidgetType.Fader, color: "#DF171A", binding: "Volume", minimumWidth: 40,  minimumHeight: 80 },
                ],
            },
        ],
    },
});
