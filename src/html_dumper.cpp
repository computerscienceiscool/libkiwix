#include "html_dumper.h"
#include "libkiwix-resources.h"
#include "tools/otherTools.h"
#include "tools.h"
#include "tools/regexTools.h"
#include "server/i18n.h"

namespace kiwix
{

/* Constructor */
HTMLDumper::HTMLDumper(const Library* library, const NameMapper* nameMapper)
  : LibraryDumper(library, nameMapper)
{
}
/* Destructor */
HTMLDumper::~HTMLDumper()
{
}

namespace {

std::string humanFriendlyTitle(std::string title)
{
  std::string humanFriendlyString = replaceRegex(title, "_", " ");
  humanFriendlyString[0] = toupper(humanFriendlyString[0]);
  return humanFriendlyString;
}

kainjow::mustache::list getTagList(std::string tags)
{
  const auto tagsList = kiwix::split(tags, ";", true, false);
  kainjow::mustache::list finalTagList;
  for (auto tag : tagsList) {
  if (tag[0] != '_')
    finalTagList.push_back(kainjow::mustache::object{
      {"tag", tag}
    });
  }
  return finalTagList;
}

} // unnamed namespace

std::string HTMLDumper::dumpPlainHTML(kiwix::Filter filter) const
{
  kainjow::mustache::list booksData;
  const auto filteredBooks = library->filter(filter);
  const auto searchQuery = filter.getQuery();
  auto languages = getLanguageData();
  auto categories = getCategoryData();

  for (auto &category : categories) {
    const auto categoryName = category.get("name")->string_value();
    if (categoryName == filter.getCategory()) {
      category["selected"] = true;
    }
    category["hf_name"] = humanFriendlyTitle(categoryName);
  }

  for (auto &language : languages) {
    if (language.get("lang_code")->string_value() == filter.getLang()) {
      language["selected"] = true;
    }
  }

  for ( const auto& bookId : filteredBooks ) {
    const auto bookObj = library->getBookById(bookId);
    const auto bookTitle = bookObj.getTitle();
    std::string contentId = "";
    try {
      contentId = urlEncode(nameMapper->getNameForId(bookId));
    } catch (...) {}
    const auto bookDescription = bookObj.getDescription();
    const auto langCode = bookObj.getCommaSeparatedLanguages();
    const auto bookIconUrl = rootLocation + "/catalog/v2/illustration/" + bookId +  "/?size=48";
    const auto tags = bookObj.getTags();
    const auto downloadAvailable = (bookObj.getUrl() != "");
    std::string faviconAttr = "style=background-image:url(" + bookIconUrl + ")";
    
    booksData.push_back(kainjow::mustache::object{
      {"id", contentId},
      {"title", bookTitle},
      {"description", bookDescription},
      {"langCode", langCode},
      {"faviconAttr", faviconAttr},
      {"tagList", getTagList(tags)},
      {"downloadAvailable", downloadAvailable}
    });
  }

  auto getTranslation = i18n::GetTranslatedStringWithMsgId(m_userLang);

  const auto translations = kainjow::mustache::object{
                    getTranslation("search"),
                    getTranslation("download"),
                    getTranslation("count-of-matching-books", {{"COUNT", to_string(filteredBooks.size())}}),
                    getTranslation("book-filtering-all-categories"),
                    getTranslation("book-filtering-all-languages"),
                    getTranslation("powered-by-kiwix-html"),
                    getTranslation("welcome-to-kiwix-server"),
                    getTranslation("preview-book")
  };

  return render_template(
             RESOURCE::templates::no_js_library_page_html,
             kainjow::mustache::object{
               {"root", rootLocation},
               {"books", booksData },
               {"searchQuery", searchQuery},
               {"languages", languages},
               {"categories", categories},
               {"translations", translations}
             }
  );
}

} // namespace kiwix
