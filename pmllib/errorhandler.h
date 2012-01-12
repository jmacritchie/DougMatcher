
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_USE

class ErrorHandler : public DOMErrorHandler {

 public:
  ErrorHandler();
  bool handleError( const DOMError &domError );
  ~ErrorHandler();
};

