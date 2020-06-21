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

void curl_data(std::string url, std::string *readBuffer)
{
  CURL *curl = curl_easy_init();
  CURLcode res;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, readBuffer);

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
}

void parse_links(std::string readBuffer, std::vector<std::string> *req_links)
{
  GumboOutput *output = gumbo_parse(readBuffer.c_str());
  search_for_links(output->root, req_links);
  gumbo_destroy_output(&kGumboDefaultOptions, output);
}

void get_first_five_para(GumboNode *node, std::string *text)
{
  /*
  if (node->type = GUMBO_NODE_TEXT)
  {
    std::string s = node->v.text.text;
    if (!s.empty())
      std::cout << s;
  }
  */
  if (node->v.element.tag == GUMBO_TAG_B)
  {
    std::cout << node->v.text.text;
  }
  else if (node->type == GUMBO_NODE_ELEMENT)
  {
    GumboVector *children = &node->v.element.children;
    //std::cout << "Not the element you were looking for\n";
    for (unsigned i = 0; i < children->length; i++)
      get_first_five_para(static_cast<GumboNode *>(children->data[i]), text);
  }
  else
  {
    return;
  }
  /*
  GumboText *data;

  GumboVector *children = &node->v.element.children;
  //std::cout << "Not the element you were looking for\n";
  for (unsigned i = 0; i < children->length; i++)
    get_first_five_para(static_cast<GumboNode *>(children->data[i]), text);
    */
}

static std::string cleantext(GumboNode *node)
{
  if (node->type == GUMBO_NODE_TEXT)
  {
    return std::string(node->v.text.text);
  }
  else if (node->type == GUMBO_NODE_ELEMENT &&
           node->v.element.tag != GUMBO_TAG_SCRIPT &&
           node->v.element.tag != GUMBO_TAG_STYLE && node->v.element.tag != GUMBO_TAG_TABLE && node->v.element.attributes.)
  {
    std::string contents = "";
    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i)
    {
      const std::string text = cleantext((GumboNode *)children->data[i]);
      if (i != 0 && !text.empty())
      {
        contents.append(" ");
      }
      contents.append(text);
    }
    if (contents.length() > 180)
      std::cout << contents << std::endl
                << std::endl;
    return contents;
  }
  else
  {
    return "";
  }
}

void get_text(std::string readBuffer, std::string *text)
{
  GumboOutput *output = gumbo_parse(readBuffer.c_str());
  get_first_five_para(output->document, text);
  gumbo_destroy_output(&kGumboDefaultOptions, output);
}

int main()
{

  // go to wikipedia main page and get html
  std::string main_url = "https://en.wikipedia.org/wiki/Main_Page";
  std::string readBuffer;

  curl_data(main_url, &readBuffer);

  // parse data using html-parser library gumbo-parser
  // get links for required articles
  std::vector<std::string> req_links;

  parse_links(readBuffer, &req_links);

  // for each article, get the first five paragraphs
  // temp url
  std::string tempbuffer;
  std::string temptext;
  std::string temp_url = "https://en.wikipedia.org/wiki/High_Explosive_Research";
  curl_data(temp_url, &tempbuffer);
  // get_text(tempbuffer, &temptext);

  GumboOutput *output = gumbo_parse(tempbuffer.c_str());
  std::string op = cleantext(output->root);
  gumbo_destroy_output(&kGumboDefaultOptions, output);
  // std::cout << op << std::endl;

  /*
  for (std::string article_url : req_links)
  {
    std::string articleBuffer;
    std::string text;
    curl_data(article_url, &articleBuffer);
    get_text(articleBuffer, &text);
  }
  */

  // check if lenght is greater than something
  // add in database
}
