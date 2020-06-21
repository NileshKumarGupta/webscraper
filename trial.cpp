#include <iostream>
#include <bits/stdc++.h>
#include <curl/curl.h>
#include "gumbo.h"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

static void search_for_links(GumboNode *node, std::vector<std::string> *req_links)
{
  if (node->type != GUMBO_NODE_ELEMENT)
    return;

  GumboAttribute *href;

  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href")))
  {
    std::string temp = href->value;
    // get all wiki links
    std::string pre = "https://en.wikipedia.org";
    std::string ff = temp.substr(0, 5);
    if (!ff.compare("/wiki"))
      req_links->push_back(pre + temp);
  }

  GumboVector *children = &node->v.element.children;
  for (unsigned i = 0; i < children->length; i++)
    search_for_links(static_cast<GumboNode *>(children->data[i]), req_links);
}

int main()
{

  // go to wikipedia main page and get html
  CURL *curl = curl_easy_init();
  std::string main_url = "https://en.wikipedia.org/wiki/Main_Page";
  CURLcode res;
  std::string readBuffer;

  std::cout << "Hello World\n";

  curl_easy_setopt(curl, CURLOPT_URL, main_url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  // parse data using html-parser library gumbo-parser
  std::vector<std::string> req_links;
  GumboOutput *output = gumbo_parse(readBuffer.c_str());
  search_for_links(output->root, &req_links);

  std::cout << "Value in req_links are :\n";
  for (std::string s : req_links)
    std::cout << s << std::endl;

  gumbo_destroy_output(&kGumboDefaultOptions, output);
}
