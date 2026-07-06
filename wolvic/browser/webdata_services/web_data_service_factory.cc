#include "wolvic/browser/webdata_services/web_data_service_factory.h"

#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/no_destructor.h"
#include "components/os_crypt/async/browser/key_provider.h"
#include "components/os_crypt/async/browser/os_crypt_async.h"
#include "components/webdata_services/web_data_service_wrapper.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"

// This class is derived from //chrome/webdata_service/WebDataServiceFactory.

namespace wolvic {

namespace {

// Callback to show error dialog on context load error.
void ContextErrorCallback(WebDataServiceWrapper::ErrorType error_type,
                          sql::InitStatus status,
                          const std::string& diagnostics) {
  // TODO(jfernandez): Implement this error management callback.
}

std::unique_ptr<KeyedService> BuildWebDataService(
    content::BrowserContext* context) {
  const base::FilePath& path = context->GetPath();
  // M132 made an OSCryptAsync mandatory for WebDataServiceWrapper (its
  // LoadDatabase dereferences it). The WebData databases delegate all
  // encryption to OSCrypt, so — like android_webview — we use an OSCryptAsync
  // with no key providers. It is process-global and must outlive every
  // WebDataServiceWrapper KeyedService, hence the function-local static.
  static base::NoDestructor<os_crypt_async::OSCryptAsync> os_crypt(
      std::vector<
          std::pair<size_t, std::unique_ptr<os_crypt_async::KeyProvider>>>{});
  return std::make_unique<WebDataServiceWrapper>(
      path, "" /* application locale */,
      content::GetUIThreadTaskRunner({}),
      base::BindRepeating(&ContextErrorCallback), os_crypt.get());
}

} // namespace

WebDataServiceFactory::WebDataServiceFactory() = default;

WebDataServiceFactory::~WebDataServiceFactory() = default;

// static
WebDataServiceWrapper* WebDataServiceFactory::GetForContext(
    content::BrowserContext* context,
    ServiceAccessType access_type) {
  return GetForBrowserContext(context, access_type);
}

// static
WebDataServiceWrapper* WebDataServiceFactory::GetForContextIfExists(
    content::BrowserContext* context,
    ServiceAccessType access_type) {
  return GetForBrowserContextIfExists(context, access_type);
}

// static
WebDataServiceFactory* WebDataServiceFactory::GetInstance() {
  static base::NoDestructor<WebDataServiceFactory> instance;
  return instance.get();
}

content::BrowserContext* WebDataServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  // Create a separate instance of the service for the Incognito context.
  return context;
}

std::unique_ptr<KeyedService>
WebDataServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return BuildWebDataService(context);
}

bool WebDataServiceFactory::ServiceIsNULLWhileTesting() const {
  return false;
}

} // wolvic
