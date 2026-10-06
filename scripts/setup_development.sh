#!/bin/bash

# nur.h Development Environment Setup Script
# Sets up complete development environment for nur.h library

set -e

echo "🚀 Setting up nur.h development environment..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if we're in the right directory
if [ ! -f "src/nur.h" ]; then
    echo -e "${RED}❌ Error: src/nur.h not found. Run this script from the nur.h project root.${NC}"
    exit 1
fi

echo -e "${BLUE}📁 Project structure verified${NC}"

# Check for required tools
echo -e "${YELLOW}🔧 Checking development tools...${NC}"

check_tool() {
    if command -v "$1" &> /dev/null; then
        echo -e "${GREEN}✅ $1 found${NC}"
        return 0
    else
        echo -e "${RED}❌ $1 not found${NC}"
        return 1
    fi
}

# Essential tools
MISSING_TOOLS=0
check_tool "gcc" || MISSING_TOOLS=$((MISSING_TOOLS + 1))
check_tool "clang" || echo -e "${YELLOW}⚠️  clang recommended but not required${NC}"
check_tool "make" || MISSING_TOOLS=$((MISSING_TOOLS + 1))

# Optional but recommended tools
check_tool "valgrind" || echo -e "${YELLOW}⚠️  valgrind recommended for memory testing${NC}"
check_tool "cppcheck" || echo -e "${YELLOW}⚠️  cppcheck recommended for static analysis${NC}"
check_tool "git" || echo -e "${YELLOW}⚠️  git recommended for version control${NC}"

if [ $MISSING_TOOLS -gt 0 ]; then
    echo -e "${RED}❌ Missing $MISSING_TOOLS essential tools. Please install them first.${NC}"
    exit 1
fi

# Run initial compilation test
echo -e "${YELLOW}🔨 Testing compilation...${NC}"
if gcc -std=c11 -Wall -Wextra -Wpedantic -fsyntax-only src/nur.h; then
    echo -e "${GREEN}✅ Header compiles successfully${NC}"
else
    echo -e "${RED}❌ Compilation failed${NC}"
    exit 1
fi

# Build and run tests
echo -e "${YELLOW}🧪 Building and running tests...${NC}"
if make test; then
    echo -e "${GREEN}✅ All tests pass${NC}"
else
    echo -e "${RED}❌ Tests failed${NC}"
    exit 1
fi

# Run static analysis if available
if command -v cppcheck &> /dev/null; then
    echo -e "${YELLOW}🔍 Running static analysis...${NC}"
    if make analyze; then
        echo -e "${GREEN}✅ Static analysis clean${NC}"
    else
        echo -e "${YELLOW}⚠️  Static analysis warnings found${NC}"
    fi
fi

# Setup git hooks if git is available and .git exists
if [ -d ".git" ] && command -v git &> /dev/null; then
    echo -e "${YELLOW}🪝 Setting up git hooks...${NC}"
    
    # Pre-commit hook
    cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# nur.h pre-commit hook
echo "Running pre-commit checks..."
make check || exit 1
make test || exit 1
echo "Pre-commit checks passed!"
EOF
    chmod +x .git/hooks/pre-commit
    echo -e "${GREEN}✅ Git pre-commit hook installed${NC}"
fi

# Create IDE configuration hints
echo -e "${YELLOW}⚙️  Creating IDE configuration...${NC}"

# VS Code settings
mkdir -p .vscode
cat > .vscode/c_cpp_properties.json << 'EOF'
{
    "configurations": [
        {
            "name": "nur.h",
            "includePath": ["${workspaceFolder}/src"],
            "defines": [],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c11",
            "intelliSenseMode": "gcc-x64",
            "compilerArgs": [
                "-Wall",
                "-Wextra", 
                "-Wpedantic"
            ]
        }
    ],
    "version": 4
}
EOF

cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build tests",
            "type": "shell",
            "command": "make",
            "args": ["test"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "run static analysis",
            "type": "shell", 
            "command": "make",
            "args": ["analyze"],
            "group": "build"
        }
    ]
}
EOF

echo -e "${GREEN}✅ VS Code configuration created${NC}"

# Summary
echo -e "\n${GREEN}🎉 Development environment setup complete!${NC}"
echo -e "\n${BLUE}📋 Quick Start Commands:${NC}"
echo -e "  ${YELLOW}make test${NC}        - Run test suite"
echo -e "  ${YELLOW}make check${NC}       - Check compilation"
echo -e "  ${YELLOW}make analyze${NC}     - Run static analysis" 
echo -e "  ${YELLOW}make dev${NC}         - Full development check"
echo -e "  ${YELLOW}make clean${NC}       - Clean build artifacts"
echo -e "\n${BLUE}📁 Key Files:${NC}"
echo -e "  ${YELLOW}src/nur.h${NC}       - Main library header"
echo -e "  ${YELLOW}test/test_nur.c${NC}  - Test suite"
echo -e "  ${YELLOW}examples/${NC}        - Usage examples"
echo -e "  ${YELLOW}docs/${NC}            - Documentation"

echo -e "\n${GREEN}Happy coding! 🚀${NC}"