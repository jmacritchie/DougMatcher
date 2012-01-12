Analysis::Analysis( PMLDocument *doc, ScorePart *part ){
    m_doc = doc;

    DOMDocument *domdoc = part->getElement()->getOwnerDocument();
    m_scorePart = part;
    m_element = domdoc->createElement( XS("perfpart") );

  //create part attribute
    m_element->setAttribute( XS("part"), XS( m_scorePart->getID().c_str() ) );
  
  //append part to performance element
    DOMElement *perf = (DOMElement*)domdoc->getElementsByTagName(XS("performance"))->item(0);
    perf->appendChild( m_element );

    DOMNodeList *nl = m_element->getElementsByTagName(XS(PNote::Tag));

    for( int i=0; i<nl->getLength(); i++ ){
        m_notes.push_back( new PNote( m_doc, (DOMElement*)nl->item(i) ) );

    }

}

void PNote::pitch_trajectory( SNote *note, bool correct ){

    //checks for an align tag within the pnote 'event' tag (m_element)
    //if one doesn't exist it is added as a child of the 'event' tag
    DOMNodeList *list = m_element->getElementsByTagName(XS("trajectory"));
    DOMElement *align;

    if( list->getLength() == 0 ){
        DOMDocument *doc = m_element->getOwnerDocument();
        align = doc->createElement(XS("align"));
        m_element->appendChild(align);
    }
    else{
        align = (DOMElement*)list->item(0);
    }
    //'align' tag attributes then set as - note="snote id"
    align->setAttribute( XS(ALIGN_TAG), XS(note->getID().c_str()) );
		       
    if( correct )
        setText( align, "correct" );
    else
        setText( align, "wrong" );

}


void PNote::align( SNote *note, bool correct ){

    //checks for an align tag within the pnote 'event' tag (m_element)
    //if one doesn't exist it is added as a child of the 'event' tag
    DOMNodeList *list = m_element->getElementsByTagName(XS("align"));
    DOMElement *align;

    if( list->getLength() == 0 ){
        DOMDocument *doc = m_element->getOwnerDocument();
        align = doc->createElement(XS("align"));
        m_element->appendChild(align);
    }
    else{
        align = (DOMElement*)list->item(0);
    }
    //'align' tag attributes then set as - note="snote id"
    align->setAttribute( XS(ALIGN_TAG), XS(note->getID().c_str()) );
		       
    if( correct )
        setText( align, "correct" );
    else
        setText( align, "wrong" );

}

Guesture::Guesture(){
    m_document = 
    m_element = 
            
}
void Guesture::setPitchTraj(string trajectory_list){
    
    setSubElementText(m_element, const "trajectory", trajectory_list);
    
    
}