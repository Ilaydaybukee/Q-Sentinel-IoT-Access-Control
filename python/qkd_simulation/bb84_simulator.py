"""Simple BB84/QKD-inspired simulator for Q-Sentinel.

This is not real quantum communication. It is a software demo that helps explain
how a shared key could be generated conceptually before protecting IoT messages.
"""

import random
from dataclasses import dataclass


@dataclass
class BB84Result:
    sifted_key: str
    qber: float
    secure: bool


def random_bits(n: int) -> list[int]:
    return [random.randint(0, 1) for _ in range(n)]


def random_bases(n: int) -> list[str]:
    return [random.choice(['+', 'x']) for _ in range(n)]


def simulate_bb84(n: int = 64, eavesdropper_rate: float = 0.0) -> BB84Result:
    alice_bits = random_bits(n)
    alice_bases = random_bases(n)
    bob_bases = random_bases(n)

    bob_bits = []
    for bit, a_base, b_base in zip(alice_bits, alice_bases, bob_bases):
        if random.random() < eavesdropper_rate:
            bit = random.randint(0, 1)
        if a_base == b_base:
            bob_bits.append(bit)
        else:
            bob_bits.append(random.randint(0, 1))

    sifted_alice = []
    sifted_bob = []
    for a_bit, b_bit, a_base, b_base in zip(alice_bits, bob_bits, alice_bases, bob_bases):
        if a_base == b_base:
            sifted_alice.append(a_bit)
            sifted_bob.append(b_bit)

    if not sifted_alice:
        return BB84Result('', 1.0, False)

    errors = sum(a != b for a, b in zip(sifted_alice, sifted_bob))
    qber = errors / len(sifted_alice)
    key = ''.join(str(bit) for bit in sifted_alice)
    secure = qber < 0.11

    return BB84Result(key, qber, secure)


if __name__ == '__main__':
    result = simulate_bb84(n=128, eavesdropper_rate=0.02)
    print('Sifted key:', result.sifted_key)
    print('QBER:', round(result.qber * 100, 2), '%')
    print('Secure:', result.secure)
