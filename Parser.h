#pragma once

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <cctype>
#include <stack>
#include <unordered_map>
#include <set>
#include <functional>
#include <iomanip>
#include <climits>

struct Node {
    std::string value;
    int index;
    Node* left = nullptr;
    Node* right = nullptr;

    Node( std::string v, int i ) : value( trim( v ) ), index( i ) {}

    static std::string trim( std::string s ) {
        s.erase( s.find_last_not_of( ' ' ) + 1 );
        s.erase( 0, s.find_first_not_of( ' ' ) );
        return s;
    }
};

class Parser {
    std::vector<Node*> nodes;
    std::string expr;
    Node* root = nullptr;

    int getPrecedence( const std::string& op ) {
        if ( op == "!" ) return 4;
        if ( op == "&&" ) return 3;
        if ( op == "||" || op == "|" ) return 2;
        if ( op == "->" ) return 1;
        if ( op == "<->" ) return 0;
        return -1;
    }

    Node* buildTree( std::vector<Node*>& tokens, int start, int end ) {
        if ( start > end ) return nullptr;
        if ( start == end ) return tokens[start];

        int minPrec = INT_MAX, opIndex = -1, balance = 0;
        for ( int i = start; i <= end; ++i ) {
            if ( tokens[i]->value == "(" ) balance++;
            else if ( tokens[i]->value == ")" ) balance--;
            else if ( balance == 0 ) {
                int prec = getPrecedence( tokens[i]->value );
                if ( prec >= 0 && prec <= minPrec ) minPrec = prec, opIndex = i;
            }
        }

        if ( opIndex == -1 ) {
            if ( tokens[start]->value == "(" && tokens[end]->value == ")" )
                return buildTree( tokens, start + 1, end - 1 );
            return tokens[start];
        }

        Node* n = tokens[opIndex];
        if ( n->value == "!" ) n->right = buildTree( tokens, opIndex + 1, end );
        else {
            n->left = buildTree( tokens, start, opIndex - 1 );
            n->right = buildTree( tokens, opIndex + 1, end );
        }
        return n;
    }

    void tokenize( const std::string& s, std::vector<Node*>& out ) {
        for ( int i = 0; i < s.size();) {
            if ( s[i] == ' ' ) { ++i; continue; }
            if ( s.substr( i, 3 ) == "<->" ) out.push_back( new Node( "<->", i ) ), i += 3;
            else if ( s.substr( i, 2 ) == "->" ) out.push_back( new Node( "->", i ) ), i += 2;
            else if ( s.substr( i, 2 ) == "&&" ) out.push_back( new Node( "&&", i ) ), i += 2;
            else if ( s.substr( i, 2 ) == "||" ) out.push_back( new Node( "||", i ) ), i += 2;
            else if ( s[i] == '!' ) out.push_back( new Node( "!", i ) ), i++;
            else if ( s[i] == '(' || s[i] == ')' ) out.push_back( new Node( std::string( 1, s[i] ), i ) ), i++;
            else if ( std::isalpha( s[i] ) ) out.push_back( new Node( std::string( 1, s[i] ), i ) ), i++;
            else ++i;
        }
    }

    bool evaluate( Node* node, const std::unordered_map<char, bool>& vars ) {
        if ( !node ) return false;
        if ( node->value == "!" ) return !evaluate( node->right, vars );
        if ( node->value == "&&" ) return evaluate( node->left, vars ) && evaluate( node->right, vars );
        if ( node->value == "||" || node->value == "|" ) return evaluate( node->left, vars ) || evaluate( node->right, vars );
        if ( node->value == "->" ) return !evaluate( node->left, vars ) || evaluate( node->right, vars );
        if ( node->value == "<->" ) return evaluate( node->left, vars ) == evaluate( node->right, vars );
        if ( vars.count( node->value[0] ) ) return vars.at( node->value[0] );
        return false;
    }

    std::vector<char> getVars() {
        std::set<char> s;
        for ( auto* n : nodes ) if ( n->value.size() == 1 && std::isalpha( n->value[0] ) ) s.insert( n->value[0] );
        return std::vector<char>( s.begin(), s.end() );
    }

public:
    Parser( std::string s ) : expr( s ) {}

    void parse() {
        std::vector<Node*> tokens;
        tokenize( expr, tokens );
        nodes = tokens;
        root = buildTree( tokens, 0, tokens.size() - 1 );
    }

    void eval() {
        if ( !root ) return;
        auto vars = getVars();
        std::vector<std::pair<std::string, Node*>> parts;
        std::set<Node*> seen;

        std::function<void( Node* )> walk = [&] ( Node* n ) {
            if ( !n || seen.count( n ) ) return;
            seen.insert( n );
            walk( n->left );
            walk( n->right );
            if ( ( n->value.size() > 1 || !std::isalpha( n->value[0] ) ) && n != root ) {
                std::string txt;
                if ( n->left && n->right ) txt = "(" + n->left->value + " " + n->value + " " + n->right->value + ")";
                else if ( n->right ) txt = n->value + n->right->value;
                else txt = n->value;
                parts.emplace_back( txt, n );
            }
            };
        walk( root );

        for ( char c : vars ) std::cout << " " << c << " |";
        for ( auto& [s, _] : parts ) std::cout << " " << std::setw( s.size() ) << s << " |";
        std::cout << " " << expr << "\n";

        int w = vars.size() * 4;
        for ( auto& [s, _] : parts ) w += s.size() + 3;
        w += expr.size() + 1;
        std::cout << std::string( w, '-' ) << "\n";

        int total = 1 << vars.size();
        for ( int i = total - 1; i >= 0; --i ) {
            std::unordered_map<char, bool> ctx;
            for ( int j = 0; j < vars.size(); ++j ) {
                bool bit = ( i >> ( vars.size() - j - 1 ) ) & 1;
                ctx[vars[j]] = bit;
                std::cout << " " << ( bit ? 'V' : 'F' ) << " |";
            }
            for ( auto& [s, n] : parts ) {
                bool b = evaluate( n, ctx );
                std::cout << " " << std::setw( s.size() ) << ( b ? 'V' : 'F' ) << " |";
            }
            std::cout << " " << std::setw( expr.size() ) << ( evaluate( root, ctx ) ? 'V' : 'F' ) << "\n";
        }
    }

    ~Parser() {
        for ( auto n : nodes ) delete n;
    }
};
