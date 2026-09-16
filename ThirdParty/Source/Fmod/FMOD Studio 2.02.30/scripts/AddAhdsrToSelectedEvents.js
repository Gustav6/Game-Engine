/* -------------------------------------------
   FMOD Studio Script Example:
   Add AHDSR to Selected Events
   -------------------------------------------
 */

studio.menu.addMenuItem({ name: "FMOD Examples\\Add AHDSR to Selected Events",
    isEnabled: function() { return studio.window.browserSelection().length; },
    
    execute: function() {
    
        function findEventsRecursively(items, filter) {
            var result = [];
            items.forEach(function(item) {
                if(item.isOfExactType("Event")) {
                    result.push(item);
                }
                if(item.isOfType("Folder")) {
                    result = result.concat(findEventsRecursively(item.items));
                }
            });
            return result;
        }
    
        function updateAHDSR(widget, commit) {
            var findStr = widget.findWidget("m_filterText").text();
            var findRecursively = widget.findWidget("m_findEventsRecursively").isChecked();
            
            var allEvents = studio.window.browserSelection();
            if(findRecursively)
                allEvents = findEventsRecursively(allEvents);
            
            // Generate preview and optionally commit changes
            var previewStr = "";
            allEvents.forEach(function(event) {
                var matchesFilter = findStr.length ? new RegExp(RegExp.escape(findStr), 'gi').test(event.name) : true;
                if(event.isOfExactType("Event") && matchesFilter) {
                    var masterBus = event.mixer.masterBus;
                    var existingAdsr = false;
                    masterBus.modulators.forEach(function(modulator) {
                        if(modulator.entity === "ADSRModulator" && modulator.nameOfPropertyBeingModulated === "volume") {
                            existingAdsr = true;
                        }
                    });
                    
                    if(!existingAdsr) {
                        previewStr += event.name.toHtmlEscaped().replace(/ /g, "&nbsp;") + '<font color="YellowGreen"> can be modified</font><br />';
                        
                        if(commit) {
                            var adsr = studio.project.create("ADSRModulator");
                            adsr.nameOfPropertyBeingModulated = "volume";
                            adsr.initialValue = widget.findWidget("m_initialValue").text(); // dB
                            adsr.peakValue = widget.findWidget("m_peakValue").text(); // dB
                            adsr.sustainValue = widget.findWidget("m_sustainValue").text(); // dB
                            adsr.attackTime = widget.findWidget("m_attackTime").text(); // time in ms
                            adsr.releaseTime = widget.findWidget("m_releaseTime").text(); // time in ms
                            adsr.holdTime = widget.findWidget("m_holdTime").text(); // time in ms
                            adsr.decayTime = widget.findWidget("m_decayTime").text(); // time in ms
                            adsr.objectBeingModulated = masterBus;
                            masterBus.uiModulationDrawerVisible = true;
                        }
                    }
                    else {
                        previewStr += event.name.toHtmlEscaped().replace(/ /g, "&nbsp;") + '<font color="Grey"> already contains an AHDSR</font><br />';
                    }
                }
            });
            if(!previewStr.length)
                previewStr = '<font color="Gray">No events are selected.</font>';
            widget.findWidget("m_preview").setHtml(previewStr);
        }
        
        studio.ui.showModalDialog({
            windowTitle: "Add AHDSR to Master",
            widgetType: studio.ui.widgetType.Layout,
            layout: studio.ui.layoutType.VBoxLayout,
            items: [
                { widgetType: studio.ui.widgetType.Label, text: "Filter Selection" },
                {
                    widgetType: studio.ui.widgetType.Layout,
                    layout: studio.ui.layoutType.VBoxLayout,
                    contentsMargins: { top: 0, bottom: 6, },
                    items: [
                        {
                            widgetType: studio.ui.widgetType.LineEdit,
                            widgetId: "m_filterText",
                            text: "",
                            onTextEdited: function() { updateAHDSR(this); },
                        },
                        {
                            widgetType: studio.ui.widgetType.CheckBox,
                            widgetId: "m_findEventsRecursively",
                            text: "Find Events Recursively",
                            isChecked: false,
                            onToggled: function() { updateAHDSR(this); },
                        },
                    ],
                },
                { widgetType: studio.ui.widgetType.Label, text: "Envelope" },
                {
                    widgetType: studio.ui.widgetType.Layout,
                    layout: studio.ui.layoutType.GridLayout,
                    contentsMargins: { top: 0, bottom: 6, },
                    items: [
                        { row: 0, column: 0, widgetType: studio.ui.widgetType.Label, text: "Initial Level (dB)" },
                        { row: 0, column: 1, widgetType: studio.ui.widgetType.LineEdit, widgetId: "m_initialValue", text: "-80", onTextEdited: function() { updateAHDSR(this); } },
                        { row: 0, column: 2, widgetType: studio.ui.widgetType.Label, text: "Peak Level (dB)" },
                        { row: 0, column: 3, widgetType: studio.ui.widgetType.LineEdit, widgetId: "m_peakValue", text: "0", onTextEdited: function() { updateAHDSR(this); } },
                        { row: 1, column: 0, widgetType: studio.ui.widgetType.Label, text: "Sustain Level (dB)" },
                        { row: 1, column: 1, widgetType: studio.ui.widgetType.LineEdit, widgetId: "m_sustainValue", text: "0", onTextEdited: function() { updateAHDSR(this); } },
                        { row: 2, column: 0, widgetType: studio.ui.widgetType.Label, text: "Attack Time (ms)" },
                        { row: 2, column: 1, widgetType: studio.ui.widgetType.LineEdit, widgetId: "m_attackTime", text: "1000", onTextEdited: function() { updateAHDSR(this); } },
                        { row: 2, column: 2, widgetType: studio.ui.widgetType.Label, text: "Release Time (ms)" },
                        { row: 2, column: 3, widgetType: studio.ui.widgetType.LineEdit, widgetId: "m_releaseTime", text: "1000", onTextEdited: function() { updateAHDSR(this); } },
                        { row: 3, column: 0, widgetType: studio.ui.widgetType.Label, text: "Hold Time (ms)" },
                        { row: 3, column: 1, widgetType: studio.ui.widgetType.LineEdit, widgetId: "m_holdTime", text: "1000", onTextEdited: function() { updateAHDSR(this); } },
                        { row: 3, column: 2, widgetType: studio.ui.widgetType.Label, text: "Decay Time (ms)" },
                        { row: 3, column: 3, widgetType: studio.ui.widgetType.LineEdit, widgetId: "m_decayTime", text: "1000", onTextEdited: function() { updateAHDSR(this); } },
                    ],
                },
                { widgetType: studio.ui.widgetType.Label, text: "Preview" },
                {
                    widgetType: studio.ui.widgetType.TextEdit,
                    widgetId: "m_preview",
                    html: '<font color="Gray">Edit envelope parameters to preview which events will be modified. You can use a <i>filter</i> select events containing specific text.',
                    isReadOnly: true,
                },
                {
                    widgetType: studio.ui.widgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
                    contentsMargins: { left: 0, top: 12, right: 0, bottom: 0 },
                    items: [
                        { widgetType: studio.ui.widgetType.Spacer, sizePolicy: { horizontalPolicy: studio.ui.sizePolicy.MinimumExpanding } },
                        { widgetType: studio.ui.widgetType.PushButton, text: "Apply", onClicked: function() { updateAHDSR(this, true); this.closeDialog(); } },
                    ],
                },
            ],
        });
    },
});
