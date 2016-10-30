function checkEnterRestoreOrig(e, cgiBase, cgiName){ //e is event object passed from function invocation
    var characterCode //literal character code will be stored in this variable
      
	if(e && e.which){ //if which property of event object is supported (NN4)
	    e = e
		characterCode = e.which //character code is contained in NN4's which property
		}
	else{
	    e = event
		characterCode = e.keyCode //character code is contained in IE's keyCode property
		}
      
    if(characterCode == 13){ //if generated character code is equal to ascii 13 (if enter key)
	//document.forms[1].submit() //submit the form
	window.open(cgiBase +
		    cgiName + '?xmlFileName=' + document.PepXMLViewerForm.xmlFileName.value,'_top')
	    return false
	    }
    else{
	return true
	    }
      
}

function checkEnter(e){ //e is event object passed from function invocation
    var characterCode //literal character code will be stored in this variable
      
	if(e && e.which){ //if which property of event object is supported (NN4)
	    e = e
		characterCode = e.which //character code is contained in NN4's which property
		}
	else{
	    e = event
		characterCode = e.keyCode //character code is contained in IE's keyCode property
		}
      
    if(characterCode == 13){ //if generated character code is equal to ascii 13 (if enter key)
	document.PepXMLViewerForm.submit() //submit the form
	    return false
	    }
    else{
	return true
	    }
      
}






function copyRadioValues(fromObj,toObj) {
    if (!fromObj || !toObj) {
	return;
    }
    var fromLen = fromObj.length;
    var toLen = toObj.length;
    if (fromLen != toLen) {
	return;
    }
    for (var c = 0; c < fromLen; c++) {
	toObj[c].checked = fromObj[c].checked;
    }
}





function copySelectValues(fromObj,toObj) {
    if (!fromObj || !toObj) {
	return;
    }
    var fromLen = fromObj.length;
    var toLen = toObj.length;
    if (fromLen != toLen) {
	return;
    }
    for (var c = 0; c < fromLen; c++) {
	toObj[c].selected = fromObj[c].selected;
    }
}





function showPane(divId) {
    var tabs = new Array();
    tabs[0] = "summaryDiv";
    tabs[1] = "displayOptionsDiv";
    tabs[2] = "columnsOptionsDiv";
    tabs[3] = "filteringOptionsDiv";
    tabs[4] = "otherActionsDiv";

    // turn off all tabs			  
    var x;
    for (x in tabs) {
	if (document.getElementById(tabs[x]+"_tab"))
	    document.getElementById(tabs[x] + "_tab").className = "banner_cid";
    
	// hide all panes
	if (document.getElementById(tabs[x]))
	    document.getElementById(tabs[x]).className="hideit";
    }

    // highlight just the selected tab and show the selected pane
    if (document.getElementById(divId)) {
	document.getElementById(divId).className="formentry";
	document.getElementById(divId+"_tab").className="formtabON";

	// store the currently displayed state
	document.PepXMLViewerForm.displayState.value = divId;
    }
    else {
	// store the currently displayed state
	document.PepXMLViewerForm.displayState.value = 'none';
    }
    
    return true;
}




function showdiv(div_id,nothidden){
    if (document.getElementById(div_id).className == nothidden) {
	new_state = 'hideit';
	document.PepXMLViewerForm.displayState.value = "";
    } else {
	new_state = nothidden;
	document.PepXMLViewerForm.displayState.value = div_id;
    }
    document.getElementById(div_id).className = new_state;
}


function hilight(tr_id,yesno) {
    if (yesno == 'yes') {
	document.getElementById(tr_id).className = 'navselected';
    } else {
	document.getElementById(tr_id).className = '';
    }
}
