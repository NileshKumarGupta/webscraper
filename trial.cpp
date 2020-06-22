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

    if (!ff.compare("/wiki") && temp.find(':') == std::string::npos && temp.find("List") == std::string::npos && temp.find("Deaths") == std::string::npos)
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

std::string cleantext(GumboNode *node, std::string *textBuffer)
{
  if (node->type == GUMBO_NODE_TEXT)
  {
    return std::string(node->v.text.text);
  }
  else if (node->type == GUMBO_NODE_ELEMENT &&
           node->v.element.tag != GUMBO_TAG_SCRIPT &&
           node->v.element.tag != GUMBO_TAG_STYLE &&
           node->v.element.tag != GUMBO_TAG_TABLE)
  {
    std::string contents = "";
    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i)
    {
      const std::string text = cleantext((GumboNode *)children->data[i], textBuffer);
      if (i != 0 && !text.empty())
      {
        contents.append(" ");
      }
      contents.append(text);
    }
    if (contents.length() > 180)
      textBuffer->append(contents + "\n\n");
    return contents;
  }
  else
  {
    textBuffer->append("");
    return "";
  }
}

void get_text(std::string readBuffer, std::vector<std::string> *all_para, std::string *textBuffer)
{
  GumboOutput *output = gumbo_parse(readBuffer.c_str());
  std::string text;
  // get the cleaned text
  text = cleantext(output->root, textBuffer);

  // get the first para as where the first newline is encountered
  unsigned cut_pos = textBuffer->find('\n');
  std::string req_text = textBuffer->substr(0, cut_pos);
  // remove all references found in para
  std::string editedtext;
  bool brfound = false;
  for (char s : req_text)
  {
    if (s == '[')
      brfound = true;
    if (brfound)
    {
      if (s == ']')
        brfound = false;
    }
    else
      editedtext += s;
  }
  // check if somehow the para have lots of links
  bool valid = true;
  for (char s : editedtext)
  {

    if (!std::isalnum(s))
      if (!(s == ' ' || s == ',' || s == '.' || s == '(' || s == ')' || s == '-' || s == '_' || s == '&' || s == '\'' || s == '\"'))
      {
        valid = false;
        break;
      }
  }
  if (valid)
    all_para->push_back(editedtext);
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

  for (std::string s : req_links)
    std::cout << s << std::endl;

  std::vector<std::string> all_para;
  // get all para's
  for (int i = 5; i < req_links.size() - 5; i++)
  {
    std::string article_url = req_links[i];
    std::string articleBuffer;
    std::string text;
    curl_data(article_url, &articleBuffer);
    get_text(articleBuffer, &all_para, &text);
  }

  for (std::string s : all_para)
  {
    std::cout << s << std::endl;
  }
  // add in database
}
