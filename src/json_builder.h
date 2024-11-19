#include "json.h"

namespace json {

// Начните проектирование кода с описания ожидаемого состояния класса (в первую очередь — последней вершины в стеке) для каждой точки в цепочке вызовов в примере.

class Builder {

    class BaseContext;
    class DictItemContext;
    class DictValueContext;
    class ArrayItemContext;



  private:
    json::Node root_{nullptr};
    std::vector<Node*> nodes_stack_; // стек указателей на те вершины JSON, которые ещё не построены: то есть текущее описываемое значение и цепочка его родителей. Он поможет возвращаться в нужный контекст после вызова End-методов.

  public:
    Builder();
    // Key(std::string). При определении словаря задаёт строковое значение ключа для очередной пары ключ-значение. Следующий вызов метода обязательно должен задавать соответствующее этому ключу значение с помощью метода Value или начинать его определение с помощью StartDict или StartArray.
    DictValueContext Key(std::string);

    // Value(Node::Value). Задаёт значение, соответствующее ключу при определении словаря, очередной элемент массива или, если вызвать сразу после конструктора json::Builder, всё содержимое конструируемого JSON-объекта. Может принимать как простой объект — число или строку — так и целый массив или словарь.
    // Здесь Node::Value — это синоним для базового класса Node, шаблона variant с набором возможных типов-значений. Смотрите заготовку кода.
    BaseContext Value(json::Node::Value);

    // StartDict(). Начинает определение сложного значения-словаря. Вызывается в тех же контекстах, что и Value. Следующим вызовом обязательно должен быть Key или EndDict.
    DictItemContext StartDict();

    // StartArray(). Начинает определение сложного значения-массива. Вызывается в тех же контекстах, что и Value. Следующим вызовом обязательно должен быть EndArray или любой, задающий новое значение: Value, StartDict или StartArray.
    ArrayItemContext StartArray();

    // EndDict(). Завершает определение сложного значения словаря. Последним незавершённым вызовом Start* должен быть StartDict.
     BaseContext EndDict();

    // EndArray(). Завершает определение сложного значения-массива. Последним незавершённым вызовом Start* должен быть StartArray.
    Builder::BaseContext EndArray();

    // Build(). Возвращает объект json::Node, содержащий JSON, описанный предыдущими вызовами методов. К этому моменту для каждого Start* должен быть вызван соответствующий End*. При этом сам объект должен быть определён, то есть вызов json::Builder{}.Build() недопустим.
    json::Node Build();

    // Описанный синтаксис позволяет указывать ключи словаря в определённом порядке. Тем не менее, в данном случае это учитывать не нужно. Словари всё так же должны храниться с помощью контейнера map.

    // При реализации обратите внимание на метод emplace_back у вектора: в отличие от push_back он принимает не сам добавляемый объект, а аргументы конструктора этого объекта. Иногда это может быть удобно.

private:
  class BaseContext {
  public:
  friend class DictValueContext;
  friend class ArrayItemContext;

    explicit BaseContext(Builder &);
    Node Build();
    DictValueContext Key(std::string);
    BaseContext Value(Node::Value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();

  private:
    Builder &builder_;
  }; // class BaseContext

//класс для пары
  class DictItemContext : public BaseContext {
  public:
    DictItemContext(BaseContext base) : BaseContext(base){};
    Node Build() = delete;
    auto Value(Node::Value value) = delete;
    auto StartDict() = delete;
    auto StartArray() = delete;
    auto EndArray() = delete;
  }; // DictItemContext

//класс для значения (словарь map)
    class DictValueContext : public BaseContext {
    public:
      DictValueContext(BaseContext base) : BaseContext(base) {}
      Node Build() = delete;
      auto Key()= delete;
      DictItemContext Value(Node::Value);
      auto EndDict() = delete;
      auto EndArray() = delete;
    }; // DictValueContext

    class ArrayItemContext : public BaseContext {
    public:
      ArrayItemContext(BaseContext base) : BaseContext(base) {}
      Node Build() = delete;
      ArrayItemContext Value(Node::Value);
      auto Key() = delete;
      auto EndDict() = delete;
    }; // ArrayItemContext
};
}  // namespace json