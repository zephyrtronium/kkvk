// Minimal XML parsing required for the Vulkan spec.
// Notably, it does not handle namespaces, CDATA, most entity codes, or XML comments.
module vulkan/xml/xml

import std/core/string
import std/text/parse

pub effect xml
  fun begin-tag(elem: string, attrs: list<attr>): ()
  fun xml-text(text: string): ()
  fun end-tag(elem: string): ()

pub struct attr
  key: string
  value: string

pub fun attr/show(v: attr): string
  v.key ++ "=\"" ++ v.value.replace-all("\"", "&quot;") ++ "\""

pub fun attr/(==)(a: attr, b: attr): bool
  a.key == b.key && a.value == b.value

val ws = whitespace

fun pattr(): parse attr
  ws()
  val k = many1(alpha-num)
  ws()
  char('=')
  ws()
  val v = choose([
    pquot('"'),
    pquot('\''),
    fn() many1(alpha-num),
  ])
  Attr(k.string, unquote(v.string))

fun pquot(q: char): (() -> parse list<char>)
  fn()
    char(q)
    val l = chars-are("string", fn(c) c != q)
    char(q)
    l

fun entity/unquote(s: string): string
  if !s.contains("&") then return s
  val qs = [
    ("&gt;", ">"),
    ("&lt;", "<"),
    ("&quot;", "\""),
    ("&amp;", "&"), // must be last so that &amp;quot; unquotes to &quot; instead of "
  ]
  qs.foldl(s, fn(u, (f, r)) u.replace-all(f, r))

type tag
  Open
    name: string
    attrs: list<attr>
  SelfClose
    name: string
    attrs: list<attr>
  Close
    name: string

fun tag/show(t: tag): string
  match t
    Open(name, attrs) -> "<" ++ name ++ attrs.map(fn(a) " " ++ a.show).join("") ++ ">"
    SelfClose(name, attrs) -> "<" ++ name ++ attrs.map(fn(a) " " ++ a.show).join("") ++ "/>"
    Close(name) -> "</" ++ name ++ ">"

fun ptag(): parse tag
  ws()
  char('<')
  ws()
  choose([
    {
      char('/')
      ws()
      val e = many1(alpha-num).string
      ws()
      char('>')
      Close(e)
    },
    {
      val e = many1(alpha-num).string
      val a = many(pattr)
      ws()
      val s = one-of("/>")
      if s == '/' then
        char('>')
        SelfClose(e, a)
      else
        Open(e, a)
    },
  ])

fun ptext(): parse string
  ws()
  chars-are("text", fn(c) c != '<').string.trim

fun pdirective(): parse string
  ws()
  pstring("<?")
  val t = none-of-many1("?")
  pstring("?>")
  t

pub fun xml/stream(s: sslice): <xml,exn,div> ()
  parse(s)
    many(pdirective) // skip directives
    match ptag()
      Close(name) -> throw("document begins with close tag", MismatchedTag("", name))
      SelfClose(name, attrs) ->
        begin-tag(name, attrs)
        end-tag(name)
      Open(name, attrs) ->
        begin-tag(name, attrs)
        stream-content(name)
  ()

tail fun stream-content(started: string): <xml,parse,exn,div> ()
  val s = ptext()
  if s.is-notempty then xml-text(s)
  match ptag()
    Close(name) ->
      if name != started then throw("mismatched open and close tags", MismatchedTag(started, name))
      end-tag(started)
    SelfClose(name, attrs) ->
      begin-tag(name, attrs)
      end-tag(name)
      stream-content(started)
    Open(name, attrs) ->
      begin-tag(name, attrs)
      stream-content(name)

extend type exception-info
  MismatchedTag(open: string, close: string)