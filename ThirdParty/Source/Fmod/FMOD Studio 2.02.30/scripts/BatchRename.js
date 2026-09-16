/* -------------------------------------------
   FMOD Studio Script Example:
   Batch Rename Dialog
   -------------------------------------------
 */

studio.menu.addMenuItem({
    name: "FMOD Examples\\Batch Rename",
    isEnabled: function() { return studio.window.browserSelection().length; },
    execute: function() {
    
        function doBatchRename(widget, commit) {
            var action = widget.findWidget("m_action").currentIndex();            
            var findStr = widget.findWidget("m_findText").text();
            var replaceStr = widget.findWidget("m_replaceText").text();
            var occurrenceType = widget.findWidget("m_occurrence").currentIndex();
            var caseSensitive = widget.findWidget("m_caseSensitive").isChecked();
            var useRegex = widget.findWidget("m_regexEnabled").isChecked();
            
            // Change the action label, or hide when we're just changing case
            switch(action) {
                case 0: widget.findWidget("m_actionLabel").setText("Replace with"); break;
                case 1: widget.findWidget("m_actionLabel").setText("Append with"); break;
                case 2: widget.findWidget("m_actionLabel").setText("Prepend with"); break;
            }
            widget.findWidget("m_findLabel").setText(action === 0 ? "Find" : "Filter");
            widget.findWidget("m_occurrenceLabel").setVisible(action === 0);
            widget.findWidget("m_occurrence").setVisible(action === 0);            
            
            // Build up the regex
            var regexStr = "";
            var regexFlags = caseSensitive ? '' : 'i';
            if(findStr.length) {
                regexStr = useRegex ? findStr : RegExp.escape(findStr);
                if(action === 0) { // Occurrence type is only applicable to the Replace action
                    switch(occurrenceType) {
                        case 0: regexFlags += 'g'; break; // All
                        case 2: regexStr = regexStr + '(?!.*' + regexStr + ')'; break; // Last
                    }
                }
            }
            
            // Generate preview and optionally commit changes
            var previewStr = "";
            try {
                var regex = regexStr.length ? new RegExp(regexStr, regexFlags) : null;
                studio.window.browserSelection().forEach(function(item) {
                    if(item.isOfType("WorkspaceItem") || item.isOfType("Asset")) {
                        var oldName;
                        if(item.isOfType("Asset")) {
                            oldName = item.getAssetPath();
                        }
                        else {
                            oldName = item.name;
                        }
                        var newName = oldName;
                        switch(action) {
                            case 0: newName = regex ? newName.replace(regex, replaceStr) : newName; break; // Replace
                            case 1: newName = !regex || newName.search(regex) != -1 ? newName + replaceStr : newName; break; // Append
                            case 2: newName = !regex || newName.search(regex) != -1 ? replaceStr + newName : newName; break; // Prepend
                        }
                        newName = newName.trim();
                        
                        if(oldName == newName) {
                            previewStr += newName.toHtmlEscaped().replace(/ /g, "&nbsp;") + '<font color="Gray"> unchanged</font><br />';
                        }
                        else {
                            previewStr += oldName.toHtmlEscaped().replace(/ /g, "&nbsp;");
                            previewStr += '<font color="Gray"> to </font><font color="YellowGreen">';
                            previewStr += newName.toHtmlEscaped().replace(/ /g, "&nbsp;") + '</font><br />';
                        }
                        if(commit) {
                            if(item.isOfType("Asset")) {
                                item.setAssetPath(newName);
                            }
                            else {
                                item.name = newName;
                            }
                        }
                    }
                });
            }
            catch(e) {
                previewStr += '<font color="Red">Invalid regular expression specified.</font>';
            }
            widget.findWidget("m_preview").setHtml(previewStr);
        }
        
        studio.ui.showModalDialog({
            windowTitle: "Batch Rename",
            windowWidth: 340,
            widgetType: studio.ui.widgetType.Layout,
            layout: studio.ui.layoutType.VBoxLayout,
            items: [
                { widgetType: studio.ui.widgetType.Label, text: "Operation" },
                {
                    widgetType: studio.ui.widgetType.ComboBox,
                    widgetId: "m_action",
                    items: [
                        { text: "Replace" }, // 0
                        { text: "Append" },  // 1
                        { text: "Prepend" }, // 2
                    ],
                    currentIndex: 0,
                    onCurrentIndexChanged: function() { doBatchRename(this); },
                },
                { widgetType: studio.ui.widgetType.Label, widgetId: "m_findLabel", text: "Find" },
                {
                    widgetType: studio.ui.widgetType.LineEdit,
                    widgetId: "m_findText",
                    text: "",
                    onTextEdited: function() { doBatchRename(this); },
                },
                { widgetType: studio.ui.widgetType.Label, widgetId: "m_actionLabel", text: "Replace with" },
                {
                    widgetType: studio.ui.widgetType.LineEdit,
                    widgetId: "m_replaceText",
                    text: "",
                    regExpValidator: "^[^<>:\\\\\/\"|*?]*$",
                    onTextEdited: function() { doBatchRename(this); },
                },
                { widgetType: studio.ui.widgetType.Label, widgetId: "m_occurrenceLabel", text: "Occurrence" },
                {
                    widgetType: studio.ui.widgetType.ComboBox,
                    widgetId: "m_occurrence",
                    items: [
                        { text: "All" },   // 0
                        { text: "First" }, // 1
                        { text: "Last" },  // 2
                    ],
                    currentIndex: 0,
                    onCurrentIndexChanged: function() { doBatchRename(this); },
                },
                {
                    widgetType: studio.ui.widgetType.CheckBox,
                    widgetId: "m_caseSensitive",
                    text: "Case sensitive",
                    isChecked: false,
                    onToggled: function() { doBatchRename(this); },
                },
                {
                    widgetType: studio.ui.widgetType.CheckBox,
                    widgetId: "m_regexEnabled",
                    text: "Use regular expressions",
                    isChecked: false,
                    onToggled: function() { doBatchRename(this); },
                },
                { widgetType: studio.ui.widgetType.Label, text: "Preview" },
                {
                    widgetType: studio.ui.widgetType.TextEdit,
                    widgetId: "m_preview",
                    html: '<font color="Gray">Enter parameters to preview changes. Use <i>$n</i> within the replacement text to use captured groups.</font>',
                    isReadOnly: true,
                },
                {
                    widgetType: studio.ui.widgetType.Layout,
                    layout: studio.ui.layoutType.HBoxLayout,
                    contentsMargins: { left: 0, top: 12, right: 0, bottom: 0 },
                    items: [
                        { widgetType: studio.ui.widgetType.Spacer, sizePolicy: { horizontalPolicy: studio.ui.sizePolicy.MinimumExpanding } },
                        { widgetType: studio.ui.widgetType.PushButton, text: "Apply", onClicked: function() { doBatchRename(this, true); this.closeDialog(); } },
                    ],
                },
            ],
        });
    },
});
