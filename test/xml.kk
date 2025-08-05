import std/core/sslice
import std/test
import vulkan/xml/xml

fun main() { run-tests(suite) }

type observe
  Open(name: string, attrs: list<attr>)
  Text(text: string)
  Close(name: string)

// Automatically generated.
// Equality comparison of the `observe` type.
fun observe/(==)(this : observe, other : observe) : e bool
  match (this, other)
    (Open(name, attrs), Open(name', attrs')) -> name == name' && attrs == attrs'
    (Text(text), Text(text')) -> text == text'
    (Close(name), Close(name')) -> name == name'
    (_, _) -> False

fun observe/show(a: observe): string
  match a
    Open(name, attrs) -> "> " ++ name ++ " " ++ attrs.map(show).join(" ")
    Text(text) -> ": " ++ text
    Close(name) -> "< " ++ name

fun collect(doc: sslice)
  var l := ctx hole
  with handler
    fun begin-tag(name, attrs)
      println("begin collecting " ++ name)
      l := l ++ ctx Cons(Open(name, attrs), hole)
    fun end-tag(name)
      println("end collecting " ++ name)
      l := l ++ ctx Cons(Close(name), hole)
    fun xml-text(text)
      println("text collecting " ++ text)
      l := l ++ ctx Cons(Text(text), hole)
  stream(doc)
  l ++. Nil

fun suite()
  group("single tags")
    test("open and close")
      val doc = "<bocchi></bocchi>"
      expect([Open("bocchi", []), Close("bocchi")], ?show=list/show)
        collect(doc.slice)
