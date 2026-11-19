import type { Config } from 'tailwindcss';

// theme.extend 只读 CSS 变量，不写死色值（AGENTS.md §4.1）
export default {
  content: ['./index.html', './src/**/*.{ts,tsx}'],
  theme: {
    extend: {
      colors: {
        brand: 'var(--color-brand)',
        'brand-hover': 'var(--color-brand-hover)',
        'brand-active': 'var(--color-brand-active)',
        'brand-soft': 'var(--color-brand-soft)',
        link: 'var(--color-link)',
        app: 'var(--bg-app)',
        rail: 'var(--bg-rail)',
        'rail-hover': 'var(--bg-rail-hover)',
        'rail-active': 'var(--bg-rail-active)',
        list: 'var(--bg-list)',
        'list-hover': 'var(--bg-list-hover)',
        'list-active': 'var(--bg-list-active)',
        panel: 'var(--bg-panel)',
        header: 'var(--bg-header)',
        input: 'var(--bg-input)',
        tooltip: 'var(--bg-tooltip)',
        line: 'var(--line)',
        'line-light': 'var(--line-light)',
        primary: 'var(--text-primary)',
        secondary: 'var(--text-secondary)',
        tertiary: 'var(--text-tertiary)',
        inverse: 'var(--text-inverse)',
        'on-brand': 'var(--text-on-brand)',
        danger: 'var(--color-danger)',
        warning: 'var(--color-warning)',
        info: 'var(--color-info)',
        success: 'var(--color-success)',
      },
      backgroundColor: {
        bubble: {
          self: 'var(--bubble-self)',
          other: 'var(--bubble-other)',
        },
      },
      textColor: {
        bubble: 'var(--bubble-text)',
      },
      borderRadius: {
        base: 'var(--radius-base)',
        sm: 'var(--radius-sm)',
        lg: 'var(--radius-lg)',
        pill: 'var(--radius-pill)',
      },
      fontFamily: {
        sans: ['var(--font-family)'],
      },
      fontSize: {
        xs: ['var(--font-size-xs)', 'var(--line-height)'],
        sm: ['var(--font-size-sm)', 'var(--line-height)'],
        base: ['var(--font-size-base)', 'var(--line-height)'],
        lg: ['var(--font-size-lg)', 'var(--line-height)'],
        xl: ['var(--font-size-xl)', 'var(--line-height)'],
      },
      boxShadow: {
        sm: 'var(--shadow-sm)',
        popup: 'var(--shadow-popup)',
        toast: 'var(--shadow-toast)',
      },
      width: {
        rail: 'var(--rail-width)',
        list: 'var(--list-width)',
        avatar: 'var(--avatar-size)',
      },
      height: {
        rail: 'var(--rail-width)',
        header: 'var(--header-height)',
        toolbar: 'var(--toolbar-height)',
        avatar: 'var(--avatar-size)',
        badge: 'var(--badge-height)',
      },
      minHeight: {
        input: 'var(--input-min-height)',
      },
      spacing: {
        0: 'var(--space-0)',
        1: 'var(--space-1)',
        2: 'var(--space-2)',
        3: 'var(--space-3)',
        4: 'var(--space-4)',
        5: 'var(--space-5)',
        6: 'var(--space-6)',
        7: 'var(--space-7)',
        8: 'var(--space-8)',
      },
      transitionTimingFunction: {
        standard: 'var(--ease-standard)',
      },
      transitionDuration: {
        fast: 'var(--duration-fast)',
        base: 'var(--duration-base)',
      },
    },
  },
  plugins: [],
} satisfies Config;
